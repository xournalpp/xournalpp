#include <memory>
#include <mutex>
#include <vector>

#include <config-test.h>
#include <gtest/gtest.h>

#include "control/Control.h"
#include "control/xojfile/LoadHandler.h"
#include "control/xojfile/SaveHandler.h"
#include "gui/GladeSearchpath.h"
#include "model/Document.h"
#include "model/DocumentHandler.h"
#include "model/Layer.h"
#include "model/Stroke.h"
#include "model/XojPage.h"
#include "undo/DeleteUndoAction.h"
#include "undo/UndoRedoHandler.h"
#include "util/PathUtil.h"

TEST(DeleteUndoActionTest, restoresNonAdjacentStrokesAndPersistsEachHistoryState) {
    GladeSearchpath resources;
    resources.addSearchDirectory(GET_RESOURCE_TEMPLATE_FOLDER);
    Control control(nullptr, &resources);
    UndoRedoHandler history(&control);
    auto* document = control.getDocument();
    auto page = std::make_shared<XojPage>(600, 800);
    document->addPage(page);
    page->setSelectedLayerId(1);
    auto* layer = page->getSelectedLayer();
    std::vector<Element*> original;
    for (int i = 0; i < 5; ++i) {
        auto stroke = std::make_unique<Stroke>();
        stroke->setToolType(StrokeTool::PEN);
        stroke->setWidth(2.0 + i);
        stroke->setColor(Colors::red);
        stroke->setFill(-1);
        const double x = i % 2 == 0 ? 20.0 + 10 * i : 150.0 + i;
        stroke->setPointVector({{x, 0, 0.7}, {x, 30, 1.2}});
        original.push_back(stroke.get());
        layer->addElement(std::move(stroke));
    }
    std::unique_ptr<DeleteUndoAction> action;
    {
        std::unique_lock lock(*document);
        const InsertionOrderRef targets{InsertionPositionRef(original[0], 0), InsertionPositionRef(original[2], 2),
                                        InsertionPositionRef(original[4], 4)};
        action = DeleteUndoAction::deleteElements(page, layer, targets);
    }
    ASSERT_NE(action, nullptr);
    const auto revision = history.getRevision();
    history.addUndoAction(std::move(action));
    EXPECT_EQ(history.getRevision(), revision + 1);
    const auto path = Util::getTmpDirSubfolder() / "scribble-history.xopp";
    auto checkSaved = [&](size_t expected) {
        SaveHandler saver;
        saver.prepareSave(document, path);
        saver.saveTo(path);
        EXPECT_TRUE(saver.getErrorMessage().empty());
        auto reopened = LoadHandler{}.loadDocument(path);
        EXPECT_EQ(reopened->getPage(0)->getLayers().front()->getElements().size(), expected);
    };
    ASSERT_EQ(layer->getElements().size(), 2);
    EXPECT_EQ(layer->getElements()[0].get(), original[1]);
    EXPECT_EQ(layer->getElements()[1].get(), original[3]);
    checkSaved(2);
    history.undo();
    EXPECT_EQ(history.getRevision(), revision + 2);
    EXPECT_FALSE(history.canUndo());
    EXPECT_TRUE(history.canRedo());
    ASSERT_EQ(layer->getElements().size(), original.size());
    for (size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(layer->getElements()[i].get(), original[i]);
        auto* stroke = static_cast<Stroke*>(original[i]);
        EXPECT_EQ(stroke->getColor(), Colors::red);
        EXPECT_EQ(stroke->getWidth(), 2.0 + static_cast<double>(i));
        EXPECT_EQ(stroke->getPoint(0).z, 0.7);
        EXPECT_EQ(stroke->getPoint(1).z, 1.2);
    }
    checkSaved(5);
    history.redo();
    EXPECT_EQ(history.getRevision(), revision + 3);
    EXPECT_TRUE(history.canUndo());
    EXPECT_FALSE(history.canRedo());
    EXPECT_EQ(layer->getElements().size(), 2);
    checkSaved(2);
    history.undo();
    EXPECT_EQ(layer->getElements().size(), 5);
    history.clearContents();
    EXPECT_EQ(history.getRevision(), revision + 5);
    fs::remove(path);
}

TEST(DeleteUndoActionTest, validatesTheCompleteBatchBeforeMovingOwnership) {
    DocumentHandler handler;
    Document document(&handler);
    auto page = std::make_shared<XojPage>(600, 800);
    document.addPage(page);
    auto* layer = page->getSelectedLayer();
    for (int i = 0; i < 2; ++i) {
        layer->addElement(std::make_unique<Stroke>());
    }
    auto* first = layer->getElements()[0].get();
    auto* second = layer->getElements()[1].get();
    std::unique_lock lock(document);
    const std::vector<InsertionOrderRef> invalid{
            {InsertionPositionRef(first, 0), InsertionPositionRef(second, 2)},
            {InsertionPositionRef(first, 0), InsertionPositionRef(first, 0)},
            {InsertionPositionRef(first, 1)},
            {InsertionPositionRef(first, -1)},
    };
    for (const auto& targets: invalid) {
        EXPECT_EQ(DeleteUndoAction::deleteElements(page, layer, targets), nullptr);
        ASSERT_EQ(layer->getElements().size(), 2);
        EXPECT_EQ(layer->getElements()[0].get(), first);
        EXPECT_EQ(layer->getElements()[1].get(), second);
    }
}
