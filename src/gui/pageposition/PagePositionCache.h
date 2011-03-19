/*
 * Xournal++
 *
 * Cache for Query the page position handler
 *
 * @author Xournal Team
 * http://xournal.sf.net
 *
 * @license GPL
 */


#ifndef __PAGEPOSITIONCACHE_H__
#define __PAGEPOSITIONCACHE_H__

class PagePositionCache {
public:
	PagePositionCache();
	virtual ~PagePositionCache();

private:
	int ppId;

	friend class PagePositionHandler;
};

#endif /* __PAGEPOSITIONCACHE_H__ */
