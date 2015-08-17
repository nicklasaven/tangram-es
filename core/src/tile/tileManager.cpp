#include "tileManager.h"

#include "data/dataSource.h"
#include "platform.h"
#include "scene/scene.h"
#include "tile/tile.h"
#include "view/view.h"

#include "glm/gtx/norm.hpp"

#include <chrono>
#include <algorithm>

#define DBG(...) logMsg(__VA_ARGS__)
#define DBGC(...) logMsg(__VA_ARGS__)

namespace Tangram {

TileManager::TileManager()
    : m_loadPending(0) {

    // Instantiate workers
    m_workers = std::unique_ptr<TileWorker>(new TileWorker(*this, MAX_WORKERS));
}

TileManager::~TileManager() {
    if (m_workers->isRunning()) {
        m_workers->stop();
    }
    m_dataSources.clear();
    m_tileSet.clear();
}

void TileManager::tileLoaded(std::shared_ptr<TileTask> task) {
    m_workers->enqueue(std::move(task));
}

void TileManager::tileProcessed(std::shared_ptr<TileTask> task) {
    std::lock_guard<std::mutex> lock(m_readyTileMutex);
    m_readyTiles.push_back(std::move(task));
}

void TileManager::enqueueLoadTask(const TileID& tileID, const glm::dvec2& viewCenter) {
    // Keep the items sorted by distance and limit list to MAX_DOWNLOADS

    auto tileCenter = m_view->getMapProjection().TileCenter(tileID);
    double distance = glm::length2(tileCenter - viewCenter);

    bool isFull = m_loadTasks.size() == MAX_DOWNLOADS;
    if (isFull && m_loadTasks.back().first < distance) {
        return;
    }
    //bool isFull = false;

    auto iter = m_loadTasks.begin();
    while (iter != m_loadTasks.end()) {
        if (iter->first > distance) {
            break;
        }
        ++iter;
    }

    if (!isFull || iter != m_loadTasks.end()) {
        m_loadTasks.insert(iter, { distance, &tileID });
    }
    if (isFull) {
        m_loadTasks.pop_back();
    }
}

void TileManager::clearTileSet() {
    for (auto& entry : m_tileSet) {
        if (entry.second->task()) {
            entry.second->task()->cancel();
        }
    }

    m_tileSet.clear();
    m_tileCache.clear();

    m_loadPending = 0;
}

void TileManager::updateTileSet() {

    m_tileSetChanged = false;

    std::vector<TileID> removeTiles;

    if (!m_readyTiles.empty()) {
        std::lock_guard<std::mutex> lock(m_readyTileMutex);
        auto it = m_readyTiles.begin();

        while (it != m_readyTiles.end()) {
            auto& tile = *((*it)->tile);

            //if (setTileState(tile, TileState::ready)) {
            clearProxyTiles(tile, removeTiles);
            m_tileSetChanged = true;
            //}
            tile.task().reset();

            it = m_readyTiles.erase(it);
        }
    }

    const std::set<TileID>& visibleTiles = m_view->getVisibleTiles();

    glm::dvec2 viewCenter(m_view->getPosition().x, -m_view->getPosition().y);

    if (m_view->changedOnLastUpdate() || m_tileSetChanged) {

        // Loop over visibleTiles and add any needed tiles to tileSet
        auto setTilesIter = m_tileSet.begin();
        auto visTilesIter = visibleTiles.begin();

        while (visTilesIter != visibleTiles.end()) {

            auto& visTileId = *visTilesIter;
            auto& curTileId = setTilesIter == m_tileSet.end() ? NOT_A_TILE : setTilesIter->first;

            if (visTileId == curTileId) {
                auto& tile = setTilesIter->second;
                if (tile->task())
                    logMsg("await %d\n", tile->task()->awaitsLoading());

                if (tile->task() && tile->task()->awaitsLoading()) {
                    logMsg("re-enqueue\n");

                    enqueueLoadTask(visTileId, viewCenter);
                }
                tile->setVisible(true);

                // tiles in both sets match
                ++setTilesIter;
                ++visTilesIter;

            } else if (curTileId == NOT_A_TILE || visTileId < curTileId) {
                // tileSet is missing an element present in visibleTiles
                if (!addTile(visTileId))
                    enqueueLoadTask(visTileId, viewCenter);

                ++visTilesIter;
            } else {
                // visibleTiles is missing an element present in tileSet
                auto& tile = setTilesIter->second;
                tile->setVisible(false);
                if (tile->getProxyCounter() <= 0) {
                    removeTiles.push_back(tile->getID());
                }
                ++setTilesIter;
            }
        }
        while (setTilesIter != m_tileSet.end()) {
            // more visibleTiles missing an element present in tileSet
            auto& tile = setTilesIter->second;
            tile->setVisible(false);
            if (tile->getProxyCounter() <= 0) {
                removeTiles.push_back(tile->getID());
            }
            ++setTilesIter;
        }
    }

    {
        while (!removeTiles.empty()) {
            auto tileIter = m_tileSet.find(removeTiles.back());
            removeTiles.pop_back();

            if (tileIter != m_tileSet.end()) {
                if (tileIter->second->getProxyCounter() <= 0) {
                    removeTile(tileIter, removeTiles);
                }
            }
        }
    }

    {
        for (auto& entry : m_tileSet) {
            auto& tile = entry.second;
            auto tileCenter = m_view->getMapProjection().TileCenter(tile->getID());
            tile->setPriority(glm::length2(tileCenter - viewCenter));
        }
    }

    {
        for (auto& item : m_loadTasks) {
            auto& id = *item.second;
            auto it = m_tileSet.find(id);
            if (it == m_tileSet.end()) {
                continue;
            }

            auto& tile = it->second;
            tile->task()->load();

            logMsg("[%d, %d, %d] Load - %d\n", id.z, id.x, id.y, tile->task()->awaitsLoading());
        }
    }

    DBG("all:%d loading:%d pending:%d cached:%d cache: %fMB\n",
        m_tileSet.size(), m_loadTasks.size(),
        m_loadPending.load(), m_tileCache.size(),
        (double(m_tileCache.getMemoryUsage()) / (1024 * 1024)));

    m_loadTasks.clear();
}

bool TileManager::addTile(const TileID& _tileID) {
    DBG("[%d, %d, %d] Add\n", _tileID.z, _tileID.x, _tileID.y);
    auto tile = m_tileCache.get(_tileID);
    bool fromCache = false;

    if (tile) {
        DBGC("USING CACHED TILE\n");
        fromCache = true;
    }

    if (!tile) {
        tile = std::shared_ptr<Tile>(new Tile(_tileID, m_view->getMapProjection()));
        tile->task() = std::shared_ptr<TileTask>(new TileTask(*this, tile));

        //Add Proxy if corresponding proxy MapTile ready
        updateProxyTiles(*tile);
    }

    tile->setVisible(true);

    m_tileSet.emplace(_tileID, std::move(tile));

    return fromCache;
}

void TileManager::removeTile(std::map<TileID, std::shared_ptr<Tile>>::iterator& _tileIter,
                             std::vector<TileID>& _removes) {

    const TileID& id = _tileIter->first;
    auto& tile = _tileIter->second;

    DBG("[%d, %d, %d] Remove\n", id.z, id.x, id.y);

    clearProxyTiles(*tile, _removes);

    if (tile->isReady()) {
        // Add to cache
        m_tileCache.put(tile);
    } else {
        // Cancel tile loading
        tile->task()->cancel();
        tile->task().reset();
    }

    // Remove tile from set
    _tileIter = m_tileSet.erase(_tileIter);

}

void TileManager::updateProxyTiles(Tile& _tile) {
    const TileID& _tileID = _tile.getID();

    auto parentID = _tileID.getParent();

    const auto& parentTileIter = m_tileSet.find(parentID);
    if (parentTileIter != m_tileSet.end()) {
        auto& parent = parentTileIter->second;
        if (_tile.setProxy(Tile::parent)) {
            parent->incProxyCounter();
        }
        return;
    }

    // Get proxy from cache
    {
        auto parent = m_tileCache.get(parentID);
        if (parent) {
            DBGC("USE CACHED PARENT PROXY\n");

            _tile.setProxy(Tile::parent);
            parent->incProxyCounter();
            m_tileSet.emplace(parentID, std::move(parent));

            return;
        }
    }

    if (m_view->s_maxZoom > _tileID.z) {
        for (int i = 0; i < 4; i++) {
            auto childID = _tileID.getChild(i);

            const auto& childTileIter = m_tileSet.find(childID);
            if (childTileIter != m_tileSet.end()) {
                auto& child = childTileIter->second;

                if (_tile.setProxy(static_cast<Tile::ProxyID>(1 << i))) {
                    child->incProxyCounter();
                }
            } else {
                auto child = m_tileCache.get(childID);
                if (child) {
                    DBGC("USE CACHED CHILD PROXY\n");

                    _tile.setProxy(static_cast<Tile::ProxyID>(1 << i));
                    child->incProxyCounter();
                    m_tileSet.emplace(childID, std::move(child));
                }
            }
        }
    }
}

void TileManager::clearProxyTiles(Tile& _tile, std::vector<TileID>& _removes) {
    const TileID& _tileID = _tile.getID();

    // Check if parent proxy is present
    if (_tile.unsetProxy(Tile::parent)) {
        TileID parentID(_tileID.getParent());
        auto parentTileIter = m_tileSet.find(parentID);
        if (parentTileIter != m_tileSet.end()) {
            auto& parent = parentTileIter->second;
            parent->decProxyCounter();

            if (parent->getProxyCounter() == 0 && !parent->isVisible()) {
                _removes.push_back(parentID);
            }
        } else {
            DBG("ERROR: parent proxy unset but not found!\n");
        }
    }

    // Check if child proxies are present
    for (int i = 0; i < 4; i++) {
        if (_tile.unsetProxy(static_cast<Tile::ProxyID>(1 << i))) {
            TileID childID(_tileID.getChild(i));
            auto childTileIter = m_tileSet.find(childID);

            if (childTileIter != m_tileSet.end()) {
                auto& child = childTileIter->second;
                child->decProxyCounter();

                if (child->getProxyCounter() == 0 && !child->isVisible()) {
                    _removes.push_back(childID);
                }
            } else {
                DBG("ERROR: child proxy unset but not found! %d\n", i);
            }
        }
    }
}

}
