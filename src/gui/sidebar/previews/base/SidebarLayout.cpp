#include "SidebarLayout.h"

#include "SidebarPreviewBaseEntry.h"
#include "SidebarPreviewBase.h"

#include <list>

SidebarLayout::SidebarLayout()
{
}

SidebarLayout::~SidebarLayout()
{
}

class SidebarRow
{
public:
	SidebarRow(int width)
	{
		this->width = width;
		this->currentWidth = 0;
	}

	~SidebarRow()
	{
		clear();
	}

public:
	bool isSpaceFor(SidebarPreviewBaseEntry* p)
	{
		if (this->list.size() == 0)
		{
			return true;
		}

		if (this->currentWidth + p->getWidth() < width)
		{
			return true;
		}
		return false;
	}

	void add(SidebarPreviewBaseEntry* p)
	{
		this->list.push_back(p);
		this->currentWidth += p->getWidth();
	}

	void clear()
	{
		this->list.clear();
		this->currentWidth = 0;
	}

	int getCount()
	{
		return this->list.size();
	}

	int getWidth()
	{
		return this->currentWidth;
	}

	int placeAt(int y, GtkLayout* layout)
	{
		int height = 0;
		int x = 0;

		for (SidebarPreviewBaseEntry* p : this->list)
		{
			height = std::max(height, p->getHeight());
		}


		for (SidebarPreviewBaseEntry* p : this->list)
		{
			int currentY = (height - p->getHeight()) / 2;

			gtk_layout_move(layout, p->getWidget(), x, y + currentY);

			x += p->getWidth();
		}


		return height;
	}

private:
	int width;
	int currentWidth;

	std::list<SidebarPreviewBaseEntry*> list;
};

void SidebarLayout::layout(SidebarPreviewBase* sidebar)
{
	int y = 0;
	int width = 0;

	GtkAllocation alloc;

	gtk_widget_get_allocation(sidebar->scrollPreview, &alloc);

	SidebarRow row(alloc.width);

	for (SidebarPreviewBaseEntry* p : sidebar->previews)
	{
		if (row.isSpaceFor(p))
		{
			row.add(p);
		}
		else
		{
			y += row.placeAt(y, GTK_LAYOUT(sidebar->iconViewPreview));

			width = std::max(width, row.getWidth());

			row.clear();
			row.add(p);
		}
	}

	if (row.getCount() != 0)
	{
		y += row.placeAt(y, GTK_LAYOUT(sidebar->iconViewPreview));

		width = std::max(width, row.getWidth());

		row.clear();
	}

	gtk_layout_set_size(GTK_LAYOUT(sidebar->iconViewPreview), width, y);
}

