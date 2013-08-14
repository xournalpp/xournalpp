//========================================================================
//
// PopplerCache.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Koji Otani <sho@bbr.jp>
// Copyright (C) 2009 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#include "PopplerCache.h"

PopplerCacheKey::~PopplerCacheKey()
{
}

PopplerCacheItem::~PopplerCacheItem()
{
}

PopplerCache::PopplerCache(int cacheSizeA)
{
  cacheSize = cacheSizeA;
  keys = new PopplerCacheKey*[cacheSize];
  items = new PopplerCacheItem*[cacheSize];
  lastValidCacheIndex = -1;
}

PopplerCache::~PopplerCache()
{
  for (int i = 0; i <= lastValidCacheIndex; ++i) {
    delete keys[i];
    delete items[i];
  }
  delete[] keys;
  delete[] items;
}

PopplerCacheItem *PopplerCache::lookup(const PopplerCacheKey &key)
{
  if (lastValidCacheIndex < 0)
    return 0;

  if (*keys[0] == key) {
    return items[0];
  }
  for (int i = 1; i <= lastValidCacheIndex; i++) {
    if (*keys[i] == key) {
      PopplerCacheKey *keyHit = keys[i];
      PopplerCacheItem *itemHit = items[i];

      for (int j = i; j > 0; j--) {
        keys[j] = keys[j - 1];
        items[j] = items[j - 1];
      }
      
      keys[0] = keyHit;
      items[0] = itemHit;
      return itemHit;
    }
  }
  return 0;
}

void PopplerCache::put(PopplerCacheKey *key, PopplerCacheItem *item)
{
  int movingStartIndex = lastValidCacheIndex + 1;
  if (lastValidCacheIndex == cacheSize - 1) {
    delete keys[lastValidCacheIndex];
    delete items[lastValidCacheIndex];
    movingStartIndex = cacheSize - 1;
  } else {
    lastValidCacheIndex++;
  }
  for (int i = movingStartIndex; i > 0; i--) {
    keys[i] = keys[i - 1];
    items[i] = items[i - 1];
  }
  keys[0] = key;
  items[0] = item;
}

int PopplerCache::size()
{
  return cacheSize;
}

int PopplerCache::numberOfItems()
{
  return lastValidCacheIndex + 1;
}
    
PopplerCacheItem *PopplerCache::item(int index)
{
  return items[index];
}
    
PopplerCacheKey *PopplerCache::key(int index)
{
  return keys[index];
}
