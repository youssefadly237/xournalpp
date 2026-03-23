/*
 * Xournal++
 *
 * This file is part of the Xournal UnitTests
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <gtest/gtest.h>

#include "control/settings/Settings.h"
#include "util/PathUtil.h"

TEST(SettingsTest, testLoadDoesNotThrowForNonExistingFilePath) {
    Settings settings{"non-existing-file-path"};
    EXPECT_NO_THROW(settings.load());
}

// Rudimentary test for Settings save/load - very crude
TEST(SettingsTest, testReadWrite) {
    auto saveReloadTest = [&](const fs::path& dir) {
        std::cout << "Test saving in " << dir << std::endl;
        const fs::path outPath = dir / "xournalpp-test-units_Settings_testReadWrite.xml";
        if (fs::exists(outPath)) {
            std::cout << "Removing file (already exist): " << dir << std::endl;
            fs::remove(outPath);
        };

        Settings settings(outPath);
        settings.transactionStart();
        settings.setAudioDisabled(true);               // bool
        settings.setDefaultSaveName(u8"foo/bar€_%H");  // u8string
        settings.setPreferredLocale("es");             // string
        PageTemplateSettings tp;
        tp.parse("xoj/"
                 "template\ncopyLastPageSettings=false\ncopyLastPageSize=true\nsize=5.123x8.764\nbackgroundType="
                 "cµßtom\nbackgroundTypeConfig=m1=3,®ændomString=↓↓↓\nbackgroundColor=#abcdef\n");
        settings.setPageTemplateSettings(tp);                          // string
        settings.setDisplayDpi(123);                                   // int
        settings.setStabilizerDrag(3.1415);                            // double
        settings.setBackgroundColor(Color(123, 45, 67));               // Color
        settings.setColorPaletteSetting("foo/bar€_palette");           // path
        settings.setEraserVisibility(ERASER_VISIBILITY_HOVER);         // enum
        settings.setFont(XojFont{"myfontname italic 34"});             // Font
        settings.latexSettings.editorFont = XojFont{"myfonttest 52"};  // Font
        settings.setPreloadPagesAfter(145);                            // unsigned int
        // State properties
        settings.setLastSavePath("/tmp/test-save-path");
        settings.setLastOpenPath("/tmp/test-open-path");
        settings.setMainWndSize(1024, 768);
        settings.setMainWndMaximized(true);
        settings.setPresentationMode(true);
        settings.setSidebarVisible(false);
        settings.setToolbarVisible(false);
        settings.transactionEnd();  // calls save() and saveState()

        Settings loaded(outPath);
        loaded.load();

        // For each type, we test one that has been changed and one that should be default
        EXPECT_EQ(settings.isAudioDisabled(), loaded.isAudioDisabled());                                    // bool
        EXPECT_EQ(settings.isAutoloadPdfXoj(), loaded.isAutoloadPdfXoj());                                  // bool
        EXPECT_EQ(settings.getDefaultSaveName(), loaded.getDefaultSaveName());                              // u8string
        EXPECT_EQ(settings.getDefaultPdfExportName(), loaded.getDefaultPdfExportName());                    // u8string
        EXPECT_EQ(settings.getPreferredLocale(), loaded.getPreferredLocale());                              // string
        EXPECT_EQ(settings.getPageTemplateSettings(), loaded.getPageTemplateSettings());                    // string
        EXPECT_EQ(settings.getDisplayDpi(), loaded.getDisplayDpi());                                        // int
        EXPECT_EQ(settings.getAddHorizontalSpaceAmountLeft(), loaded.getAddHorizontalSpaceAmountLeft());    // int
        EXPECT_EQ(settings.getStabilizerDrag(), loaded.getStabilizerDrag());                                // double
        EXPECT_EQ(settings.getCursorHighlightBorderWidth(), loaded.getCursorHighlightBorderWidth());        // double
        EXPECT_EQ(settings.getBackgroundColor(), loaded.getBackgroundColor());                              // Color
        EXPECT_EQ(settings.getActiveSelectionColor(), loaded.getActiveSelectionColor());                    // Color
        EXPECT_EQ(settings.getColorPaletteSetting(), loaded.getColorPaletteSetting());                      // path
        EXPECT_EQ(settings.getLastOpenPath(), loaded.getLastOpenPath());                                    // path
        EXPECT_EQ(settings.getEraserVisibility(), loaded.getEraserVisibility());                            // enum
        EXPECT_EQ(settings.getActiveViewMode(), loaded.getActiveViewMode());                                // enum
        EXPECT_EQ(settings.getFont().getName(), loaded.getFont().getName());                                // Font
        EXPECT_EQ(settings.getFont().getSize(), loaded.getFont().getSize());                                // Font
        EXPECT_EQ(settings.latexSettings.editorFont.getName(), loaded.latexSettings.editorFont.getName());  // Font
        EXPECT_EQ(settings.latexSettings.editorFont.getSize(), loaded.latexSettings.editorFont.getSize());  // Font
        EXPECT_EQ(settings.getPreloadPagesAfter(), loaded.getPreloadPagesAfter());    // unsigned int
        EXPECT_EQ(settings.getPreloadPagesBefore(), loaded.getPreloadPagesBefore());  // unsigned int

        // State properties
        EXPECT_EQ(settings.getLastSavePath(), loaded.getLastSavePath());
        EXPECT_EQ(settings.getLastOpenPath(), loaded.getLastOpenPath());
        EXPECT_EQ(settings.getMainWndWidth(), loaded.getMainWndWidth());
        EXPECT_EQ(settings.getMainWndHeight(), loaded.getMainWndHeight());
        EXPECT_EQ(settings.isMainWndMaximized(), loaded.isMainWndMaximized());
        EXPECT_EQ(settings.isPresentationMode(), loaded.isPresentationMode());
        EXPECT_EQ(settings.isSidebarVisible(), loaded.isSidebarVisible());
        EXPECT_EQ(settings.isToolbarVisible(), loaded.isToolbarVisible());

        fs::remove(outPath);
        fs::remove(settings.getStateFile());
    };
    saveReloadTest(fs::temp_directory_path());
}

// Test that state file is created separately from settings file
TEST(SettingsTest, testStateFileSeparation) {
    const fs::path dir = fs::temp_directory_path();
    const fs::path outPath = dir / "xournalpp-test-units_Settings_testState.xml";

    if (fs::exists(outPath))
        fs::remove(outPath);

    Settings settings(outPath);
    const fs::path statePath = settings.getStateFile();

    if (fs::exists(statePath))
        fs::remove(statePath);

    settings.transactionStart();
    settings.setLastSavePath("/tmp/separate-state-test");
    settings.setMainWndSize(1920, 1080);
    settings.setMenubarVisible(false);

    // Layout configuration
    settings.setShowPairedPages(true);
    settings.setViewLayoutVert(true);
    settings.setViewLayoutR2L(true);
    settings.setViewLayoutB2T(true);
    settings.setViewFixedRows(true);
    settings.setViewColumns(2);
    settings.setViewRows(3);

    // Add tools data to settings to see if it gets migrated
    SElement& s = settings.getCustomStateElement("tools");
    s.setString("current", "pen");
    SElement& pen = s.child("pen");
    pen.setIntHex("color", 0xff0000);
    pen.setString("size", "MEDIUM");

    settings.transactionEnd();

    // State file should exist
    EXPECT_TRUE(fs::exists(statePath));

    // Verify state file contains state properties
    auto stateContent = Util::readString(statePath, false);
    ASSERT_TRUE(stateContent.has_value());
    EXPECT_NE(stateContent->find("lastSavePath"), std::string::npos);
    EXPECT_NE(stateContent->find("mainWndWidth"), std::string::npos);
    EXPECT_NE(stateContent->find("menubarVisible"), std::string::npos);
    EXPECT_NE(stateContent->find("showPairedPages"), std::string::npos);
    EXPECT_NE(stateContent->find("layoutVertical"), std::string::npos);
    EXPECT_NE(stateContent->find("layoutRightToLeft"), std::string::npos);
    EXPECT_NE(stateContent->find("layoutBottomToTop"), std::string::npos);
    EXPECT_NE(stateContent->find("viewFixedRows"), std::string::npos);
    EXPECT_NE(stateContent->find("numColumns"), std::string::npos);
    EXPECT_NE(stateContent->find("numRows"), std::string::npos);
    EXPECT_NE(stateContent->find("tools"), std::string::npos);
    EXPECT_NE(stateContent->find("pen"), std::string::npos);
    EXPECT_NE(stateContent->find("ff0000"), std::string::npos);

    // Verify settings file does NOT contain state properties
    auto settingsContent = Util::readString(outPath, false);
    ASSERT_TRUE(settingsContent.has_value());
    EXPECT_EQ(settingsContent->find("lastSavePath"), std::string::npos);
    EXPECT_EQ(settingsContent->find("mainWndWidth"), std::string::npos);
    EXPECT_EQ(settingsContent->find("showPairedPages"), std::string::npos);
    EXPECT_EQ(settingsContent->find("layoutVertical"), std::string::npos);
    EXPECT_EQ(settingsContent->find("tools"), std::string::npos);

    fs::remove(outPath);
    fs::remove(statePath);
}
