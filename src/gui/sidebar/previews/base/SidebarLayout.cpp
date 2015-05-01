#include "SidebarLayout.h"

#include "SidebarPreviewPage.h"
#include "SidebarPreviewBase.h"

SidebarLayout::SidebarLayout()
{
	XOJ_INIT_TYPE(SidebarLayout);

}

SidebarLayout::~SidebarLayout()
{
	XOJ_RELEASE_TYPE(SidebarLayout);
}

class SidebarRow
{
public:
	SidebarRow(int width)
	{
		this->width = width;
		this->list = NULL;
		this->currentWidth = 0;
	}

	~SidebarRow()
	{
		clear();
	}

public:
	bool isSpaceFor(SidebarPreviewPage* p)
	{
		if (this->list == NULL)
		{
			return true;
		}

		if (this->currentWidth + p->getWidth() < width)
		{
			return true;
		}
		return false;
	}

	void add(SidebarPreviewPage* p)
	{
		this->list = g_list_append(this->list, p);
		this->currentWidth += p->getWidth();
	}

	void clear()
	{
		g_list_free(this->list);
		this->list = NULL;
		this->currentWidth = 0;
	}

	int getCount()
	{
		return g_list_length(this->list);
	}

	int getWidth()
	{
		return this->currentWidth;
	}

	int placeAt(int y, GtkLayout* layout)
	{
		int height = 0;
		int x = 0;

		for (GList* l = this->list; l != NULL; l = l->next)
		{
			SidebarPreviewPage* p = (SidebarPreviewPage*) l->data;
			height = MAX(height, p->getHeight());
		}


		for (GList* l = this->list; l != NULL; l = l->next)
		{
			SidebarPreviewPage* p = (SidebarPreviewPage*) l->data;

			int currentY = (height - p->getHeight()) / 2;

			gtk_layout_move(layout, p->getWidget(), x, y + currentY);

			x += p->getWidth();
		}


		return height;
	}

private:
	int width;
	int currentWidth;

	GList* list;
};

void SidebarLayout::layout(SidebarPreviewBase* sidebar)
{
	int y = 0;
	int width = 0;

	GtkAllocation alloc;

	gtk_widget_get_allocation(sidebar->scrollPreview, &alloc);

	SidebarRow row(alloc.width);

	for (int i = 0; i < sidebar->previewCount; i++)
	{
		SidebarPreviewPage* p = sidebar->previews[i];

		if (row.isSpaceFor(p))
		{
			row.add(p);
		}
		else
		{
			y += row.placeAt(y, GTK_LAYOUT(sidebar->iconViewPreview));

			width = MAX(width, row.getWidth());

			row.clear();
			row.add(p);
		}
	}

	if (row.getCount() != 0)
	{
		y += row.placeAt(y, GTK_LAYOUT(sidebar->iconViewPreview));

		width = MAX(width, row.getWidth());

		row.clear();
	}

	gtk_layout_set_size(GTK_LAYOUT(sidebar->iconViewPreview), width, y);
}

