/*
 * Xournal++
 *
 * Helper classes to find / select objects
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

// No include needed, this is included after PageView.h
#include <optional>

class BaseSelectObject
{
public:
	BaseSelectObject(XojPageView* view)
	 : view(view)
	{
	}

	virtual ~BaseSelectObject() = default;

public:
	virtual bool at(double x, double y)
	{
		this->x = x;
		this->y = y;

		// clear old selection anyway
		view->xournal->getControl()->clearSelection();
		matchRect = { gint(x - 10), gint(y - 10), 20, 20 };

		for (Layer* l : *view->page->getLayers())
		{
			if (view->page->isLayerVisible(l))
			{
				return checkLayer(l);
			}
		}
		return false;
	}

protected:
	bool checkLayer(Layer* l)
	{
		for (Element* e : *l->getElements())
		{
			if (e->intersectsArea(&matchRect))
			{
				if (this->checkElement(e))
				{
					return true;
				}
			}
		}
		return false;
	}

	virtual bool checkElement(Element* e) = 0;

protected:
	GdkRectangle matchRect{};
	XojPageView* view{};
	double x{0};
	double y{0};
};

class SelectObject : public BaseSelectObject
{
public:
	SelectObject(XojPageView* view)
	 : BaseSelectObject(view)
	 , strokeMatch(nullptr)
	 , elementMatch(nullptr)
	 , gap(1000000000)
	{
	}

	~SelectObject() override = default;

	bool at(double x, double y) override
	{
		BaseSelectObject::at(x, y);

		if (strokeMatch)
		{
			elementMatch = strokeMatch;
		}

		if (elementMatch)
		{
			view->xournal->setSelection(new EditSelection(view->xournal->getControl()->getUndoRedoHandler(), elementMatch, view, view->page));

			view->repaintPage();
			
			return true;
		}
		
		return false;
	}

protected:
	bool checkElement(Element* e) override
	{
		if (e->getType() == ELEMENT_STROKE)
		{
			Stroke* s = (Stroke*) e;
			double tmpGap = 0;
			if ((s->intersects(x, y, 5, &tmpGap)) && (gap > tmpGap))
			{
				gap = tmpGap;
				strokeMatch = s;
				return true;
			}
		}
		else
		{
			elementMatch = e;
			return true;
		}
		
		return false;
	}

private:
	Stroke* strokeMatch;
	Element* elementMatch;
	double gap;
};

class PlayObject : public BaseSelectObject
{
public:
	PlayObject(XojPageView* view)
	 : BaseSelectObject(view)
	 , playbackStatus()
	{
	}

	~PlayObject() override = default;

	struct Status
	{
		bool success;
		std::string filename;
	};

	std::optional<Status> playbackStatus;

public:
	bool at(double x, double y) override
	{
		return BaseSelectObject::at(x, y);
	}

protected:
	bool checkElement(Element* e) override
	{
		if (e->getType() != ELEMENT_STROKE && e->getType() != ELEMENT_TEXT)
		{
			return false;
		}

		AudioElement* s = (AudioElement*) e;
		double tmpGap = 0;
		if ((s->intersects(x, y, 15, &tmpGap)))
		{
			size_t ts = s->getTimestamp();

			string fn = s->getAudioFilename();

			if (!fn.empty())
			{
				if (fn.rfind(G_DIR_SEPARATOR, 0) != 0)
				{
					Path path = Path::fromUri(view->settings->getAudioFolder());
					path /= fn;

					fn = path.str();
				}
				auto* ac = view->getXournal()->getControl()->getAudioController();
				bool success = ac->startPlayback(fn, (unsigned int) ts);
				playbackStatus = {success, fn};
				return success;
			}
		}
		return false;
	}
};



