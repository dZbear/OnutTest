#include "defines.h"
#include "menu.h"
#include "Properties.h"

#include <ActionManager.h>
#include <tinyxml2.h>

#include <unordered_set>
#include <unordered_map>
#include <Windows.h>

// Utilities
onut::ActionManager actionManager;
std::unordered_map<seed::Node*, NodeContainerRef> nodesToContainers;

// Camera
static const std::vector<Zoom> zoomLevels = {.20f, .50f, .70f, 1.f, 1.5f, 2.f, 4.f};
ZoomIndex zoomIndex = 3;
Zoom zoom = zoomLevels[zoomIndex];
Vector2 cameraPos = Vector2::Zero;

// Selection
Selection selection;
Selection cleanedUpSelection;
OAnimf dottedLineAnim = 0.f;
Gizmo gizmo;

// State
State state = State::Idle;
Vector2 cameraPosOnDown;
onut::sUIVector2 mousePosOnDown;
Vector2 selectionCenter;
HandleIndex handleIndexOnDown;
bool isSpinning = false;
Clipboard clipboard;

void createUIStyles(onut::UIContext* pContext);
void init();
void update();
void render();

// Controls
onut::UIControl* ui_mainView = nullptr;
onut::UITreeView* ui_treeView = nullptr;
onut::UITreeViewItem* pTreeViewRoot = nullptr;

onut::UIButton* ui_btnCreateNode = nullptr;

onut::UIButton* ui_btnCreateSprite = nullptr;
onut::UITextBox* ui_txtSpriteTexture = nullptr;
onut::UIButton* ui_btnSpriteTextureBrowse = nullptr;
onut::UICheckBox* ui_chkSpriteBlends[4] = {nullptr};
onut::UICheckBox* ui_chkSpriteFilters[2] = {nullptr};

onut::UIButton* ui_btnCreateSpriteString = nullptr;
onut::UITextBox* ui_txtSpriteStringFont = nullptr;
onut::UIButton* ui_btnSpriteStringFontBrowse = nullptr;

onut::UIButton* ui_btnCreateEmitter = nullptr;
onut::UITextBox* ui_txtEmitterFx = nullptr;
onut::UIButton* ui_txtEmitterFxBrowse = nullptr;
onut::UICheckBox* ui_chkEmitterBlends[4] = {nullptr};
onut::UICheckBox* ui_chkEmitterFilters[2] = {nullptr};

onut::UIButton* ui_btnCreateSoundEmitter = nullptr;
onut::UITextBox* ui_txtSoundEmitter = nullptr;
onut::UIButton* ui_btnSoundEmitterBrowse = nullptr;

onut::UIButton* ui_btnCreateMusicEmitter = nullptr;
onut::UITextBox* ui_txtMusicEmitter = nullptr;
onut::UIButton* ui_btnMusicEmitterBrowse = nullptr;

onut::UIButton* ui_btnCreateVideo = nullptr;
onut::UITextBox* ui_txtVideo = nullptr;
onut::UIButton* ui_btnVideoBrowse = nullptr;

onut::UIButton* ui_btnCreateEffect = nullptr;

onut::UIButton* ui_btnCreateTiledMap = nullptr;
onut::UITextBox* ui_txtTiledMap = nullptr;
onut::UIButton* ui_btnTiledMapBrowse = nullptr;

// Seed
seed::View* pEditingView = nullptr;
Vector2 viewSize = Vector2(640, 480);

// Cursors
static HCURSOR curARROW = nullptr;
static HCURSOR curSIZENWSE = nullptr;
static HCURSOR curSIZENESW = nullptr;
static HCURSOR curSIZEWE = nullptr;
static HCURSOR curSIZENS = nullptr;
static HCURSOR curSIZEALL = nullptr;

void updateTransformHandles();

std::string currentFilename;
bool isModified = false;

void markModified()
{
    isModified = true;
    OWindow->setCaption(onut::getFilename(currentFilename) + "*");
}

void markSaved()
{
    isModified = false;
    OWindow->setCaption(onut::getFilename(currentFilename));
}

// Panel for selected type
std::unordered_map<std::type_index, std::vector<onut::UIControl*>> propertiesPanels;
template<typename Tnode>
void registerPropertiesPanel(const std::string& panelName)
{
    auto pPanel = OFindUI(panelName);
    propertiesPanels[typeid(Tnode)].push_back(pPanel);
}

template<typename Ttype>
void showPropertiesPanels()
{
    auto& panels = propertiesPanels[typeid(Ttype)];
    for (auto panel : panels)
    {
        panel->isVisible = true;
    }
}

void refreshUI();
void updateProperties()
{
    refreshUI();

    ui_treeView->unselectAll();

    for (auto& kv : propertiesPanels)
    {
        for (auto panel : kv.second)
        {
            panel->isVisible = false;
        }
    }

    ui_btnCreateSpriteString->isEnabled = false;
    ui_btnCreateSprite->isEnabled = false;
    ui_btnCreateNode->isEnabled = false;
    ui_btnCreateEmitter->isEnabled = false;
    ui_btnCreateSoundEmitter->isEnabled = false;
    ui_btnCreateMusicEmitter->isEnabled = false;
    ui_btnCreateVideo->isEnabled = false;
    ui_btnCreateEffect->isEnabled = false;
    ui_btnCreateTiledMap->isEnabled = false;
    ui_mainView->isEnabled = false;

    if (!pEditingView) return;

    ui_btnCreateSpriteString->isEnabled = true;
    ui_btnCreateSprite->isEnabled = true;
    ui_btnCreateNode->isEnabled = true;
    ui_btnCreateEmitter->isEnabled = true;
    ui_btnCreateSoundEmitter->isEnabled = true;
    ui_btnCreateMusicEmitter->isEnabled = true;
    ui_btnCreateVideo->isEnabled = true;
    ui_btnCreateEffect->isEnabled = true;
    ui_btnCreateTiledMap->isEnabled = true;
    ui_mainView->isEnabled = true;

    if (selection.empty())
    {
        showPropertiesPanels<void>();
    }

    for (auto pContainer : selection)
    {
        auto pNode = pContainer->pNode;
        auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
        auto pSpriteString = dynamic_cast<seed::SpriteString*>(pContainer->pNode);
        auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
        auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
        auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
        auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
        auto pEffect = dynamic_cast<seed::Effect*>(pContainer->pNode);
        auto pTiledMapNode = dynamic_cast<seed::TiledMapNode*>(pContainer->pNode);

        showPropertiesPanels<seed::Node>();
        if (pSprite)
        {
            showPropertiesPanels<seed::Sprite>();
            auto pTexture = pSprite->GetTexture();
            if (pTexture)
            {
                ui_txtSpriteTexture->textComponent.text = pTexture->getName();
            }
            else
            {
                ui_txtSpriteTexture->textComponent.text = "";
            }
            switch (pSprite->GetBlend())
            {
                case onut::SpriteBatch::eBlendMode::Add:
                    ui_chkSpriteBlends[0]->setIsChecked(true);
                    break;
                case onut::SpriteBatch::eBlendMode::Alpha:
                    ui_chkSpriteBlends[1]->setIsChecked(true);
                    break;
                case onut::SpriteBatch::eBlendMode::PreMultiplied:
                    ui_chkSpriteBlends[2]->setIsChecked(true);
                    break;
                case onut::SpriteBatch::eBlendMode::Multiplied:
                    ui_chkSpriteBlends[3]->setIsChecked(true);
                    break;
            }
            switch (pSprite->GetFilter())
            {
                case onut::SpriteBatch::eFiltering::Nearest:
                    ui_chkSpriteFilters[0]->setIsChecked(true);
                    break;
                case onut::SpriteBatch::eFiltering::Linear:
                    ui_chkSpriteFilters[1]->setIsChecked(true);
                    break;
            }
            if (pSpriteString)
            {
                showPropertiesPanels<seed::SpriteString>();
                auto pFont = pSpriteString->GetFont();
                if (pFont)
                {
                    ui_txtSpriteStringFont->textComponent.text = pFont->getName();
                }
                else
                {
                    ui_txtSpriteStringFont->textComponent.text = "";
                }
            }
        }
        else if (pEmitter)
        {
            showPropertiesPanels<seed::Emitter>();
            ui_txtEmitterFx->textComponent.text = pEmitter->GetFxName();
            switch (pEmitter->GetBlend())
            {
                case onut::SpriteBatch::eBlendMode::Add:
                    ui_chkEmitterBlends[0]->setIsChecked(true);
                    break;
                case onut::SpriteBatch::eBlendMode::Alpha:
                    ui_chkEmitterBlends[1]->setIsChecked(true);
                    break;
                case onut::SpriteBatch::eBlendMode::PreMultiplied:
                    ui_chkEmitterBlends[2]->setIsChecked(true);
                    break;
                case onut::SpriteBatch::eBlendMode::Multiplied:
                    ui_chkEmitterBlends[3]->setIsChecked(true);
                    break;
            }
            switch (pEmitter->GetFilter())
            {
                case onut::SpriteBatch::eFiltering::Nearest:
                    ui_chkEmitterFilters[0]->setIsChecked(true);
                    break;
                case onut::SpriteBatch::eFiltering::Linear:
                    ui_chkEmitterFilters[1]->setIsChecked(true);
                    break;
            }
        }
        else if (pSoundEmitter)
        {
            showPropertiesPanels<seed::SoundEmitter>();
            ui_txtSoundEmitter->textComponent.text = pSoundEmitter->GetSource();
        }
        else if (pMusicEmitter)
        {
            showPropertiesPanels<seed::MusicEmitter>();
            ui_txtMusicEmitter->textComponent.text = pMusicEmitter->GetSource();
        }
        else if (pVideo)
        {
            showPropertiesPanels<seed::Video>();
            ui_txtVideo->textComponent.text = pVideo->GetSource();
        }
        else if (pEffect)
        {
            showPropertiesPanels<seed::Effect>();
        }
        else if (pTiledMapNode)
        {
            showPropertiesPanels<seed::TiledMapNode>();
        }

        ui_treeView->expandTo(pContainer->pTreeViewItem);
        ui_treeView->addSelectedItem(pContainer->pTreeViewItem);
    }

    void refreshUI();

    updateTransformHandles();
}

// Main
int CALLBACK WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdCount)
{
    // Set default settings
    OSettings->setGameName("Seed Editor");
    OSettings->setIsResizableWindow(true);
    OSettings->setResolution({1280, 768});
    OSettings->setIsEditorMode(true);

    // Run
    ORun(init, update, render);
}

Matrix getViewTransform()
{
    auto viewRect = ui_mainView->getWorldRect(*OUIContext);
    Matrix transform =
        Matrix::CreateTranslation(-cameraPos) *
        Matrix::CreateScale(zoom) *
        Matrix::CreateTranslation(Vector2((float)viewRect.position.x + (float)viewRect.size.x * .5f, (float)viewRect.position.y + (float)viewRect.size.y * .5f));
    return std::move(transform);
}

std::vector<Vector2> getNodeCorners(seed::Node* pNode)
{
    Matrix viewTransform = getViewTransform();
    auto spriteTransform = pNode->GetTransform();
    auto finalTransform = spriteTransform * viewTransform;
    Vector2 size = {64.f, 64.f};
    auto& align = Vector2(.5f, .5f);

    std::vector<Vector2> points = {
        Vector2::Transform(Vector2(size.x * -align.x, size.y * -align.y), finalTransform),
        Vector2::Transform(Vector2(size.x * -align.x, size.y * (1.f - align.y)), finalTransform),
        Vector2::Transform(Vector2(size.x * (1.f - align.x), size.y * (1.f - align.y)), finalTransform),
        Vector2::Transform(Vector2(size.x * (1.f - align.x), size.y * -align.y), finalTransform)
    };

    return std::move(points);
}

std::vector<Vector2> getSpriteCorners(seed::Sprite* pSprite)
{
    Matrix viewTransform = getViewTransform();
    auto spriteTransform = pSprite->GetTransform();
    auto finalTransform = spriteTransform * viewTransform;
    Vector2 size = {pSprite->GetWidth(), pSprite->GetHeight()};
    auto& align = pSprite->GetAlign();

    std::vector<Vector2> points = {
        Vector2::Transform(Vector2(size.x * -align.x, size.y * -align.y), finalTransform),
        Vector2::Transform(Vector2(size.x * -align.x, size.y * (1.f - align.y)), finalTransform),
        Vector2::Transform(Vector2(size.x * (1.f - align.x), size.y * (1.f - align.y)), finalTransform),
        Vector2::Transform(Vector2(size.x * (1.f - align.x), size.y * -align.y), finalTransform)
    };

    return std::move(points);
}

std::vector<Vector2> getVideoCorners(seed::Video* pVideo)
{
    Matrix viewTransform = getViewTransform();
    auto spriteTransform = pVideo->GetTransform();
    auto finalTransform = spriteTransform * viewTransform;
    Vector2 size = {pVideo->GetWidth(), pVideo->GetHeight()};
    Vector2 align(.5f);

    std::vector<Vector2> points = {
        Vector2::Transform(Vector2(size.x * -align.x, size.y * -align.y), finalTransform),
        Vector2::Transform(Vector2(size.x * -align.x, size.y * (1.f - align.y)), finalTransform),
        Vector2::Transform(Vector2(size.x * (1.f - align.x), size.y * (1.f - align.y)), finalTransform),
        Vector2::Transform(Vector2(size.x * (1.f - align.x), size.y * -align.y), finalTransform)
    };

    return std::move(points);
}

AABB getSelectionAABB()
{
    Matrix transform = getViewTransform();
    AABB aabb = {Vector2(100000, 100000), Vector2(-100000, -100000)};
    for (auto pContainer : selection)
    {
        auto pNode = pContainer->pNode;
        auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
        if (pSprite)
        {
            auto points = getSpriteCorners(pSprite);
            for (auto& point : points)
            {
                aabb[0] = Vector2::Min(aabb[0], point);
                aabb[1] = Vector2::Max(aabb[1], point);
            }
        }
        else
        {
            Matrix viewTransform = getViewTransform();
            auto spriteTransform = pNode->GetTransform();
            auto finalTransform = spriteTransform * viewTransform;
            auto worldPosition = finalTransform.Translation();
            aabb[0] = Vector2::Min(aabb[0], worldPosition);
            aabb[1] = Vector2::Max(aabb[1], worldPosition);
        }
    }
    return std::move(aabb);
}

bool isMultiSelection()
{
    return selection.size() > 1;
}

void updateTransformHandles()
{
    if (isMultiSelection())
    {
        gizmo.transformHandles.resize(8);
        gizmo.aabb = getSelectionAABB();

        gizmo.transformHandles[0].handle = Handle::TOP_LEFT;
        gizmo.transformHandles[0].screenPos = gizmo.aabb[0];
        gizmo.transformHandles[0].transformDirection = Vector2(-1, -1);

        gizmo.transformHandles[1].handle = Handle::LEFT;
        gizmo.transformHandles[1].screenPos = Vector2(gizmo.aabb[0].x, (gizmo.aabb[0].y + gizmo.aabb[1].y) * .5f);
        gizmo.transformHandles[1].transformDirection = Vector2(-1, 0);

        gizmo.transformHandles[2].handle = Handle::BOTTOM_LEFT;
        gizmo.transformHandles[2].screenPos = Vector2(gizmo.aabb[0].x, gizmo.aabb[1].y);
        gizmo.transformHandles[2].transformDirection = Vector2(-1, 1);

        gizmo.transformHandles[3].handle = Handle::BOTTOM;
        gizmo.transformHandles[3].screenPos = Vector2((gizmo.aabb[0].x + gizmo.aabb[1].x) * .5f, gizmo.aabb[1].y);
        gizmo.transformHandles[3].transformDirection = Vector2(0, 1);

        gizmo.transformHandles[4].handle = Handle::BOTTOM_RIGHT;
        gizmo.transformHandles[4].screenPos = gizmo.aabb[1];
        gizmo.transformHandles[4].transformDirection = Vector2(1, 1);

        gizmo.transformHandles[5].handle = Handle::RIGHT;
        gizmo.transformHandles[5].screenPos = Vector2(gizmo.aabb[1].x, (gizmo.aabb[0].y + gizmo.aabb[1].y) * .5f);
        gizmo.transformHandles[5].transformDirection = Vector2(1, 0);

        gizmo.transformHandles[6].handle = Handle::TOP_RIGHT;
        gizmo.transformHandles[6].screenPos = Vector2(gizmo.aabb[1].x, gizmo.aabb[0].y);
        gizmo.transformHandles[6].transformDirection = Vector2(1, -1);

        gizmo.transformHandles[7].handle = Handle::TOP;
        gizmo.transformHandles[7].screenPos = Vector2((gizmo.aabb[0].x + gizmo.aabb[1].x) * .5f, gizmo.aabb[0].y);
        gizmo.transformHandles[7].transformDirection = Vector2(0, -1);

        if (state != State::Rotate)
        {
            selectionCenter = (gizmo.aabb[0] + gizmo.aabb[1]) * .5f;
        }
    }
    else if (selection.empty())
    {
        gizmo.transformHandles.clear();
    }
    else
    {
        auto pNode = selection.front()->pNode;
        auto pSprite = dynamic_cast<seed::Sprite*>(pNode);
        auto pVideo = dynamic_cast<seed::Video*>(pNode);
        if (pSprite)
        {
            gizmo.transformHandles.resize(8);
            auto spriteCorners = getSpriteCorners(pSprite);

            gizmo.transformHandles[0].handle = Handle::TOP_LEFT;
            gizmo.transformHandles[0].screenPos = spriteCorners[0];
            gizmo.transformHandles[0].transformDirection = Vector2(-1, -1);

            gizmo.transformHandles[1].handle = Handle::LEFT;
            gizmo.transformHandles[1].screenPos = (spriteCorners[0] + spriteCorners[1]) * .5f;
            gizmo.transformHandles[1].transformDirection = Vector2(-1, 0);

            gizmo.transformHandles[2].handle = Handle::BOTTOM_LEFT;
            gizmo.transformHandles[2].screenPos = spriteCorners[1];
            gizmo.transformHandles[2].transformDirection = Vector2(-1, 1);

            gizmo.transformHandles[3].handle = Handle::BOTTOM;
            gizmo.transformHandles[3].screenPos = (spriteCorners[1] + spriteCorners[2]) * .5f;
            gizmo.transformHandles[3].transformDirection = Vector2(0, 1);

            gizmo.transformHandles[4].handle = Handle::BOTTOM_RIGHT;
            gizmo.transformHandles[4].screenPos = spriteCorners[2];
            gizmo.transformHandles[4].transformDirection = Vector2(1, 1);

            gizmo.transformHandles[5].handle = Handle::RIGHT;
            gizmo.transformHandles[5].screenPos = (spriteCorners[2] + spriteCorners[3]) * .5f;
            gizmo.transformHandles[5].transformDirection = Vector2(1, 0);

            gizmo.transformHandles[6].handle = Handle::TOP_RIGHT;
            gizmo.transformHandles[6].screenPos = spriteCorners[3];
            gizmo.transformHandles[6].transformDirection = Vector2(1, -1);

            gizmo.transformHandles[7].handle = Handle::TOP;
            gizmo.transformHandles[7].screenPos = (spriteCorners[3] + spriteCorners[0]) * .5f;
            gizmo.transformHandles[7].transformDirection = Vector2(0, -1);

            selectionCenter = Vector2::Transform(pSprite->GetTransform().Translation(), getViewTransform());
        }
        else if (pVideo)
        {
            gizmo.transformHandles.resize(8);
            auto spriteCorners = getVideoCorners(pVideo);

            gizmo.transformHandles[0].handle = Handle::TOP_LEFT;
            gizmo.transformHandles[0].screenPos = spriteCorners[0];
            gizmo.transformHandles[0].transformDirection = Vector2(-1, -1);

            gizmo.transformHandles[1].handle = Handle::LEFT;
            gizmo.transformHandles[1].screenPos = (spriteCorners[0] + spriteCorners[1]) * .5f;
            gizmo.transformHandles[1].transformDirection = Vector2(-1, 0);

            gizmo.transformHandles[2].handle = Handle::BOTTOM_LEFT;
            gizmo.transformHandles[2].screenPos = spriteCorners[1];
            gizmo.transformHandles[2].transformDirection = Vector2(-1, 1);

            gizmo.transformHandles[3].handle = Handle::BOTTOM;
            gizmo.transformHandles[3].screenPos = (spriteCorners[1] + spriteCorners[2]) * .5f;
            gizmo.transformHandles[3].transformDirection = Vector2(0, 1);

            gizmo.transformHandles[4].handle = Handle::BOTTOM_RIGHT;
            gizmo.transformHandles[4].screenPos = spriteCorners[2];
            gizmo.transformHandles[4].transformDirection = Vector2(1, 1);

            gizmo.transformHandles[5].handle = Handle::RIGHT;
            gizmo.transformHandles[5].screenPos = (spriteCorners[2] + spriteCorners[3]) * .5f;
            gizmo.transformHandles[5].transformDirection = Vector2(1, 0);

            gizmo.transformHandles[6].handle = Handle::TOP_RIGHT;
            gizmo.transformHandles[6].screenPos = spriteCorners[3];
            gizmo.transformHandles[6].transformDirection = Vector2(1, -1);

            gizmo.transformHandles[7].handle = Handle::TOP;
            gizmo.transformHandles[7].screenPos = (spriteCorners[3] + spriteCorners[0]) * .5f;
            gizmo.transformHandles[7].transformDirection = Vector2(0, -1);

            selectionCenter = Vector2::Transform(pVideo->GetTransform().Translation(), getViewTransform());
        }
        else
        {
            gizmo.transformHandles.resize(8);
            auto spriteCorners = getNodeCorners(pNode);

            gizmo.transformHandles[0].handle = Handle::TOP_LEFT;
            gizmo.transformHandles[0].screenPos = spriteCorners[0];
            gizmo.transformHandles[0].transformDirection = Vector2(-1, -1);

            gizmo.transformHandles[1].handle = Handle::LEFT;
            gizmo.transformHandles[1].screenPos = (spriteCorners[0] + spriteCorners[1]) * .5f;
            gizmo.transformHandles[1].transformDirection = Vector2(-1, 0);

            gizmo.transformHandles[2].handle = Handle::BOTTOM_LEFT;
            gizmo.transformHandles[2].screenPos = spriteCorners[1];
            gizmo.transformHandles[2].transformDirection = Vector2(-1, 1);

            gizmo.transformHandles[3].handle = Handle::BOTTOM;
            gizmo.transformHandles[3].screenPos = (spriteCorners[1] + spriteCorners[2]) * .5f;
            gizmo.transformHandles[3].transformDirection = Vector2(0, 1);

            gizmo.transformHandles[4].handle = Handle::BOTTOM_RIGHT;
            gizmo.transformHandles[4].screenPos = spriteCorners[2];
            gizmo.transformHandles[4].transformDirection = Vector2(1, 1);

            gizmo.transformHandles[5].handle = Handle::RIGHT;
            gizmo.transformHandles[5].screenPos = (spriteCorners[2] + spriteCorners[3]) * .5f;
            gizmo.transformHandles[5].transformDirection = Vector2(1, 0);

            gizmo.transformHandles[6].handle = Handle::TOP_RIGHT;
            gizmo.transformHandles[6].screenPos = spriteCorners[3];
            gizmo.transformHandles[6].transformDirection = Vector2(1, -1);

            gizmo.transformHandles[7].handle = Handle::TOP;
            gizmo.transformHandles[7].screenPos = (spriteCorners[3] + spriteCorners[0]) * .5f;
            gizmo.transformHandles[7].transformDirection = Vector2(0, -1);

            selectionCenter = Vector2::Transform(pNode->GetTransform().Translation(), getViewTransform());
        }
    }
}

void onFocusSelection()
{
    if (!selection.empty() &&
        state == State::Idle)
    {
        auto invViewTransform = getViewTransform().Invert();
        auto worldRect = ui_mainView->getWorldRect(*OUIContext);
        cameraPos = Vector2::Transform(selectionCenter, invViewTransform);
        updateTransformHandles();
    }
}

std::string fileOpen(const char* szFilters)
{
    auto window = OWindow->getHandle();
    char szFileName[MAX_PATH] = "";

    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = window;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window;
    ofn.lpstrFilter = szFilters;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = "json";

    GetOpenFileNameA(&ofn);

    return ofn.lpstrFile;
}

void changeSpriteProperty(const std::string& actionName, const std::function<void(std::shared_ptr<NodeContainer>)>& logic)
{
    if (isSpinning)
    {
        for (auto pContainer : selection)
        {
            logic(pContainer);
        }
        updateProperties();
        return;
    }
    auto pActionGroup = new onut::ActionGroup(actionName);
    for (auto pContainer : selection)
    {
        auto spriteStateBefore = std::make_shared<NodeState>(pContainer);
        logic(pContainer);
        auto spriteStateAfter = std::make_shared<NodeState>(pContainer);
        pActionGroup->addAction(new onut::Action("",
            [=]
        {
            spriteStateAfter->apply(); 
            updateProperties(); 
            markModified();
        },
            [=]
        {
            spriteStateBefore->apply(); 
            updateProperties(); 
            markModified();
        }));
    }
    actionManager.doAction(pActionGroup);
}

void checkAboutToAction(State aboutState, State targetState, const Vector2& mouseDiff)
{
    if (state == aboutState)
    {
        if (mouseDiff.Length() >= 3.f)
        {
            state = targetState;
            for (auto pContainer : selection)
            {
                pContainer->stateOnDown = std::make_shared<NodeState>(pContainer);
            }
        }
    }
}

void finalizeAction(const std::string& name, State actionState)
{
    if (state == actionState)
    {
        auto workSelection = selection;
        if (actionState == State::Moving ||
            actionState == State::Rotate)
        {
            workSelection = cleanedUpSelection;
        }
        auto pGroup = new onut::ActionGroup(name);
        for (auto pContainer : workSelection)
        {
            auto stateBefore = std::make_shared<NodeState>(pContainer->stateOnDown);
            auto stateAfter = std::make_shared<NodeState>(pContainer);
            pGroup->addAction(new onut::Action("",
                [=]{
                stateAfter->apply();
                updateProperties();
                markModified();
            }, [=] {
                stateBefore->apply();
                updateProperties();
                markModified();
            }));
        }
        actionManager.doAction(pGroup);
    }
}

bool mouseInSprite(const Vector2& mousePos, seed::Node* pNode)
{
    auto viewTransform = getViewTransform();
    auto transform = pNode->GetTransform() * viewTransform;
    auto invTransform = transform.Invert();
    auto mouseInSprite = Vector2::Transform(mousePos, invTransform);
    auto pSprite = dynamic_cast<seed::Sprite*>(pNode);
    auto pVideo = dynamic_cast<seed::Video*>(pNode);
    if (pSprite)
    {
        if (mouseInSprite.x >= -pSprite->GetWidth() * pSprite->GetAlign().x &&
            mouseInSprite.x <= pSprite->GetWidth() * (1.f - pSprite->GetAlign().x) &&
            mouseInSprite.y >= -pSprite->GetHeight() * pSprite->GetAlign().y &&
            mouseInSprite.y <= pSprite->GetHeight() * (1.f - pSprite->GetAlign().y))
        {
            return true;
        }
    }
    else if (pVideo)
    {
        if (mouseInSprite.x >= -pVideo->GetWidth() * .5f &&
            mouseInSprite.x <= pVideo->GetWidth() * .5f &&
            mouseInSprite.y >= -pVideo->GetHeight() * .5f &&
            mouseInSprite.y <= pVideo->GetHeight() * .5f)
        {
            return true;
        }
    }
    else
    {
        if (mouseInSprite.x >= -32 &&
            mouseInSprite.x <= 32 &&
            mouseInSprite.y >= -32 &&
            mouseInSprite.y <= 32)
        {
            return true;
        }
    }
    return false;
}

bool isMouseInSelection(const Vector2& mousePos)
{
    if (selection.empty()) return false;
    if (isMultiSelection())
    {
        return (mousePos.x >= gizmo.aabb[0].x &&
                mousePos.y >= gizmo.aabb[0].y &&
                mousePos.x <= gizmo.aabb[1].x &&
                mousePos.y <= gizmo.aabb[1].y);
    }
    else
    {
        return mouseInSprite(mousePos, selection.front()->pNode);
    }
}

bool isMouseInSelection(const onut::sUIVector2& mousePos)
{
    return isMouseInSelection(onut::UI2Onut(mousePos));
}

bool mouseInSprite(const onut::sUIVector2& mousePos, std::shared_ptr<NodeContainer> pContainer)
{
    return mouseInSprite(onut::UI2Onut(mousePos), pContainer->pNode);
}

bool mouseInSprite(const Vector2& mousePos, std::shared_ptr<NodeContainer> pContainer)
{
    return mouseInSprite(mousePos, pContainer->pNode);
}

bool mouseInSprite(const onut::sUIVector2& mousePos, seed::Node* pNode)
{
    return mouseInSprite(onut::UI2Onut(mousePos), pNode);
}

void checkNudge(uintptr_t key)
{
    // Arrow nudge
    float step = OPressed(OINPUT_LCONTROL) ? 10.f : 1.f;
    if (key == VK_LEFT)
    {
        changeSpriteProperty("Nudge", [=](std::shared_ptr<NodeContainer> pContainer)
        {
            NodeState spriteState(pContainer);
            auto worldPos = Vector2::Transform(spriteState.node.position, spriteState.node.parentTransform);
            auto invTransform = spriteState.node.parentTransform.Invert();
            worldPos.x -= step;
            pContainer->pNode->SetPosition(Vector2::Transform(worldPos, invTransform));
        });
    }
    else if (key == VK_RIGHT)
    {
        changeSpriteProperty("Nudge", [=](std::shared_ptr<NodeContainer> pContainer)
        {
            NodeState spriteState(pContainer);
            auto worldPos = Vector2::Transform(spriteState.node.position, spriteState.node.parentTransform);
            auto invTransform = spriteState.node.parentTransform.Invert();
            worldPos.x += step;
            pContainer->pNode->SetPosition(Vector2::Transform(worldPos, invTransform));
        });
    }
    if (key == VK_UP)
    {
        changeSpriteProperty("Nudge", [=](std::shared_ptr<NodeContainer> pContainer)
        {
            NodeState spriteState(pContainer);
            auto worldPos = Vector2::Transform(spriteState.node.position, spriteState.node.parentTransform);
            auto invTransform = spriteState.node.parentTransform.Invert();
            worldPos.y -= step;
            pContainer->pNode->SetPosition(Vector2::Transform(worldPos, invTransform));
        });
    }
    else if (key == VK_DOWN)
    {
        changeSpriteProperty("Nudge", [=](std::shared_ptr<NodeContainer> pContainer)
        {
            NodeState spriteState(pContainer);
            auto worldPos = Vector2::Transform(spriteState.node.position, spriteState.node.parentTransform);
            auto invTransform = spriteState.node.parentTransform.Invert();
            worldPos.y += step;
            pContainer->pNode->SetPosition(Vector2::Transform(worldPos, invTransform));
        });
    }
}

decltype(curARROW) directionCursor(decltype(curARROW) cursor, float angleDiff)
{
    auto angle = 0.f;
    if (cursor == curSIZEWE) angle = 0.f;
    if (cursor == curSIZENS) angle = -90.f;
    if (cursor == curSIZENESW) angle = 45.f;
    if (cursor == curSIZENWSE) angle = -45.f;
    angle -= angleDiff;
    while (angle < 0) angle += 360.f;
    while (angle >= 360.f) angle -= 360.f;
    if (angle < 22.5f || angle >= 337.5f) cursor = curSIZEWE;
    else if (angle < 67.5f) cursor = curSIZENESW;
    else if (angle < 112.5f) cursor = curSIZENS;
    else if (angle < 157.5f) cursor = curSIZENWSE;
    else if (angle < 202.5f) cursor = curSIZEWE;
    else if (angle < 247.5f) cursor = curSIZENESW;
    else if (angle < 292.5f) cursor = curSIZENS;
    else if (angle < 337.5f) cursor = curSIZENWSE;
    return cursor;
}

decltype(curARROW) directionCursor(decltype(curARROW) cursor, const Matrix& transform)
{
    auto right = transform.AxisX();
    return directionCursor(cursor, DirectX::XMConvertToDegrees(std::atan2f(right.y, right.x)));
}

void transformToParent(seed::Node* pParent, seed::Node* pNode)
{
    auto transform = pNode->GetTransform();
    auto targetTransform = pParent->GetTransform();

    auto worldAngle = DirectX::XMConvertToDegrees(std::atan2<>(transform.AxisX().y, transform.AxisX().x));
    auto parentWorldAngle = DirectX::XMConvertToDegrees(std::atan2<>(targetTransform.AxisX().y, targetTransform.AxisX().x));
    pNode->SetAngle(worldAngle - parentWorldAngle);

    auto worldPos = transform.Translation();
    auto localTransform = Vector2::Transform(worldPos, targetTransform.Invert());
    pNode->SetPosition(localTransform);

    auto worldScale = Vector2(transform.AxisX().Length(), transform.AxisY().Length());
    auto parentWorldScale = Vector2(targetTransform.AxisX().Length(), targetTransform.AxisY().Length());
    if (parentWorldScale.x != 0 &&
        parentWorldScale.y != 0)
    {
        pNode->SetScale(worldScale / parentWorldScale);
    }
}

void collectParentsOnly(std::shared_ptr<NodeContainer> pContainer, std::unordered_set<std::shared_ptr<NodeContainer>>& containerSet)
{
    containerSet.insert(pContainer);
    auto& bgChildren = pContainer->pNode->GetBgChildren();
    auto& fgChildren = pContainer->pNode->GetFgChildren();
    for (auto pChild : bgChildren)
    {
        auto pContainerChild = nodesToContainers[pChild];
        collectParentsOnly(pContainerChild, containerSet);
    }
    for (auto pChild : fgChildren)
    {
        auto pContainerChild = nodesToContainers[pChild];
        collectParentsOnly(pContainerChild, containerSet);
    }
}

void onDelete()
{
    if (state != State::Idle) return;

    auto oldSelection = selection;
    Selection toDeleteSelection;

    // Create a copy of all deleted item + substree
    std::unordered_set<std::shared_ptr<NodeContainer>> stateSavedContainers;
    for (auto pContainer : selection)
    {
        if (stateSavedContainers.find(pContainer) != stateSavedContainers.end()) continue; // Already in there as a child
        collectParentsOnly(pContainer, stateSavedContainers);
        toDeleteSelection.push_back(pContainer);
    }

    // Do the action
    auto pGroup = new onut::ActionGroup("Delete");

    pGroup->addAction(
        new onut::Action("",
        [=]{ // OnRedo
        selection.clear();
        updateProperties();
        markModified();
    },
        [=]{ // OnUndo
        selection = oldSelection;
        updateProperties();
        markModified();
    }));

    for (auto pContainer : toDeleteSelection)
    {
        auto pParentContainer = nodesToContainers[pContainer->pNode->GetParent()];
        std::shared_ptr<NodeContainer> pAfterSibbling = nullptr;
        auto& bgChildren = pParentContainer->pNode->GetBgChildren();
        auto& fgChildren = pParentContainer->pNode->GetFgChildren();
        for (decltype(bgChildren.size()) i = 0; i < bgChildren.size(); ++i)
        {
            if (bgChildren[i] == pContainer->pNode)
            {
                if (i < bgChildren.size() - 1)
                {
                    pAfterSibbling = nodesToContainers[bgChildren[i + 1]];
                    break;
                }
            }
        }
        if (!pAfterSibbling)
        {
            for (decltype(fgChildren.size()) i = 0; i < fgChildren.size(); ++i)
            {
                if (fgChildren[i] == pContainer->pNode)
                {
                    if (i < fgChildren.size() - 1)
                    {
                        pAfterSibbling = nodesToContainers[fgChildren[i + 1]];
                        break;
                    }
                }
            }
        }

        auto pStateBefore = std::make_shared<NodeState>(pContainer, true);

        pGroup->addAction(new onut::Action("",
            [=]{ // OnRedo
            pEditingView->DeleteNode(pContainer->pNode);
            pParentContainer->pTreeViewItem->removeItem(pContainer->pTreeViewItem);
            pStateBefore->visit([](NodeStateRef pNodeState)
            {
                pNodeState->pContainer->pNode = nullptr;
                pNodeState->pContainer->pTreeViewItem = nullptr;
            });
            pContainer->pTreeViewItem = nullptr;
            pContainer->pNode = nullptr;
            markModified();
        },
            [=]{ // OnUndo
            pStateBefore->apply(pParentContainer);
            pContainer->pTreeViewItem->setTreeView(ui_treeView);
            pParentContainer->pNode->Detach(pContainer->pNode);
            if (pAfterSibbling)
            {
                pParentContainer->pNode->AttachBefore(pContainer->pNode, pAfterSibbling->pNode);
                pParentContainer->pTreeViewItem->addItemBefore(pContainer->pTreeViewItem, pAfterSibbling->pTreeViewItem);
            }
            else
            {
                pParentContainer->pNode->Attach(pContainer->pNode);
                pParentContainer->pTreeViewItem->addItem(pContainer->pTreeViewItem);
            }
            markModified();
        }));
    }
    actionManager.doAction(pGroup);
}

void onCopy()
{
    Selection toCopySelection;
    std::unordered_set<std::shared_ptr<NodeContainer>> stateSavedContainers;
    for (auto pContainer : selection)
    {
        if (stateSavedContainers.find(pContainer) != stateSavedContainers.end()) continue; // Already in there as a child
        collectParentsOnly(pContainer, stateSavedContainers);
        toCopySelection.push_back(pContainer);
    }

    clipboard.clear();
    for (auto pContainer : toCopySelection)
    {
        NodeState nodeState(pContainer, true);
        std::shared_ptr<NodeContainer> pParentContainer = nodesToContainers[pEditingView->GetRootNode()];
        if (nodeState.pContainer->pNode)
        {
            auto pNodeParent = nodeState.pContainer->pNode->GetParent();
            if (pNodeParent)
            {
                pParentContainer = nodesToContainers[pNodeParent];
            }
        }
        nodeState.pParentContainer = pParentContainer;
        nodeState.visit([](NodeStateRef pNodeState)
        {
            pNodeState->pContainer = nullptr;
        });
        clipboard.push_back(nodeState);
    }
}

void onPaste()
{
    auto pGroup = new onut::ActionGroup("Paste");
    
    auto oldSelection = selection;
    auto copy = new Clipboard(clipboard);

    pGroup->addAction(new onut::Action("",
        [=]
    {
        for (auto& state : *copy)
        {
            state.apply(state.pParentContainer);
            state.pContainer->pTreeViewItem->setTreeView(ui_treeView);
            state.pParentContainer->pTreeViewItem->addItem(state.pContainer->pTreeViewItem);
        }
        markModified();
    }, [=]
    {
        for (auto& state : *copy)
        {
            auto pParentContainer = nodesToContainers[state.pContainer->pNode->GetParent()];
            pEditingView->DeleteNode(state.pContainer->pNode);
            pParentContainer->pTreeViewItem->removeItem(state.pContainer->pTreeViewItem);
            state.visit([](NodeStateRef pNodeState)
            {
                pNodeState->pContainer->pTreeViewItem = nullptr;
                pNodeState->pContainer->pNode = nullptr;
            });
        }
        markModified();
    }, [=]
    {
    }, [=]
    {
        delete copy;
    }));
    pGroup->addAction(new onut::Action("",
        [=]
    {
        selection.clear();
        for (auto& state : *copy)
        {
            selection.push_back(state.pContainer);
        }
        updateProperties();
        markModified();
    }, [=]
    {
        selection = oldSelection;
        updateProperties();
        markModified();
    }));

    actionManager.doAction(pGroup);
}

void onCut()
{
    onCopy();
    onDelete();
}

void onSelectAll()
{
    if (state != State::Idle) return;
    selection.clear();
    pEditingView->VisitNodes([](seed::Node* pNode) -> bool
    {
        if (pNode == pEditingView->GetRootNode()) return false;
        if (!pNode->GetReallyVisible()) return false;
        selection.push_back(nodesToContainers[pNode]);
        return false;
    });
    updateProperties();
}

void onDeselect()
{
    if (state != State::Idle) return;
    selection.clear();
    updateProperties();
}

void onSave()
{
    if (isModified && pEditingView)
    {
        pEditingView->SetSize(viewSize);
        pEditingView->Save(currentFilename);
        OSettings->setUserSetting("lastFile", currentFilename);
        markSaved();
    }
}

void onSaveAs(const std::string& filename)
{
    currentFilename = filename;
    if (pEditingView)
    {
        pEditingView->SetSize(viewSize);
        pEditingView->Save(currentFilename);
        OSettings->setUserSetting("lastFile", currentFilename);
        markSaved();
    }
}

void buildData()
{
    pEditingView->VisitNodes([](seed::Node* pNode) -> bool
    {
        auto pContainer = std::make_shared<NodeContainer>();

        pContainer->pNode = pNode;
        pContainer->pTreeViewItem = new onut::UITreeViewItem();
        pContainer->pTreeViewItem->pSharedUserData = pContainer;

        nodesToContainers[pContainer->pNode] = pContainer;
        return false;
    });

    pTreeViewRoot = nodesToContainers[pEditingView->GetRootNode()]->pTreeViewItem;
    ui_treeView->addItem(pTreeViewRoot);

    pEditingView->VisitNodes([](seed::Node* pNode) -> bool
    {
        auto pNodeParent = pNode->GetParent();
        if (pNodeParent)
        {
            auto pContainer = nodesToContainers[pNode];
            auto pParentContainer = nodesToContainers[pNodeParent];

            pParentContainer->pTreeViewItem->addItem(pContainer->pTreeViewItem);
        }
        return false;
    });

    viewSize = pEditingView->GetSize();

    updateProperties();
}

void onNew(const std::string& filename)
{
    nodesToContainers.clear();
    actionManager.clear();
    ui_treeView->clear();
    selection.clear();

    currentFilename = filename;
    OSettings->setUserSetting("lastFile", currentFilename);
    OContentManager->clearSearchPaths();
    OContentManager->addDefaultSearchPaths();
    auto path = onut::getPath(filename);
    OContentManager->addSearchPath(path);
    auto pos = path.find("assets");
    if (pos != std::string::npos)
    {
        OContentManager->addSearchPath(path.substr(0, pos) + "assets");
    }

    pEditingView = new seed::View();
    pEditingView->Show();

    buildData();
    markModified();
}

void onOpen(const std::string& filename)
{
    nodesToContainers.clear();
    actionManager.clear();
    ui_treeView->clear();
    selection.clear();

    currentFilename = filename;
    OSettings->setUserSetting("lastFile", currentFilename);
    OContentManager->clearSearchPaths();
    OContentManager->addDefaultSearchPaths();
    auto path = onut::getPath(filename);
    OContentManager->addSearchPath(path);
    auto pos = path.find("assets");
    if (pos != std::string::npos)
    {
        OContentManager->addSearchPath(path.substr(0, pos) + "assets");
    }

    pEditingView = new seed::View();
    pEditingView->Show();
    pEditingView->Load(currentFilename);

    buildData();
    markSaved();
}

// Bind toolbox actions
void createSprite(const std::string& name)
{
    if (state != State::Idle) return;

    // Undo/redo
    std::shared_ptr<NodeContainer> pContainer = std::make_shared<NodeContainer>();
    auto oldSelection = selection;
    actionManager.doAction(new onut::ActionGroup("Create Sprite",
    {
        new onut::Action("",
        [=]{ // OnRedo
            auto pSprite = pEditingView->CreateSprite(name);
            pEditingView->AddNode(pSprite);
            pSprite->SetPosition(viewSize * .5f);

            auto pTreeItem = new onut::UITreeViewItem();
            pTreeItem->pSharedUserData = pContainer;
            pTreeViewRoot->addItem(pTreeItem);

            pContainer->pNode = pSprite;
            pContainer->pTreeViewItem = pTreeItem;
            nodesToContainers[pSprite] = pContainer;
            markModified();
        },
            [=]{ // OnUndo
            auto it = nodesToContainers.find(pContainer->pNode);
            if (it != nodesToContainers.end()) nodesToContainers.erase(it);
            pEditingView->DeleteNode(pContainer->pNode);
            pTreeViewRoot->removeItem(pContainer->pTreeViewItem);
            pContainer->pTreeViewItem = nullptr;
            pContainer->pNode = nullptr;
            markModified();
        },
            [=]{ // Init
        },
            [=]{ // Destroy
        }),
            new onut::Action("",
            [=]{ // OnRedo
            selection.clear();
            selection.push_back(pContainer);
            updateProperties();
        },
            [=]{ // OnUndo
            selection = oldSelection;
            updateProperties();
        }),
    }));
}

void createEmitter(const std::string& name)
{
    if (state != State::Idle) return;

    // Undo/redo
    std::shared_ptr<NodeContainer> pContainer = std::make_shared<NodeContainer>();
    auto oldSelection = selection;
    actionManager.doAction(new onut::ActionGroup("Create Emitter",
    {
        new onut::Action("",
        [=]{ // OnRedo
            auto pEmitter = pEditingView->CreateEmitter(name);
            pEditingView->AddNode(pEmitter);
            pEmitter->SetPosition(viewSize * .5f);

            auto pTreeItem = new onut::UITreeViewItem();
            pTreeItem->pSharedUserData = pContainer;
            pTreeViewRoot->addItem(pTreeItem);

            pContainer->pNode = pEmitter;
            pContainer->pTreeViewItem = pTreeItem;
            nodesToContainers[pEmitter] = pContainer;
            markModified();
        },
            [=]{ // OnUndo
            auto it = nodesToContainers.find(pContainer->pNode);
            if (it != nodesToContainers.end()) nodesToContainers.erase(it);
            pEditingView->DeleteNode(pContainer->pNode);
            pTreeViewRoot->removeItem(pContainer->pTreeViewItem);
            pContainer->pTreeViewItem = nullptr;
            pContainer->pNode = nullptr;
            markModified();
        },
            [=]{ // Init
        },
            [=]{ // Destroy
        }),
            new onut::Action("",
            [=]{ // OnRedo
            selection.clear();
            selection.push_back(pContainer);
            updateProperties();
        },
            [=]{ // OnUndo
            selection = oldSelection;
            updateProperties();
        }),
    }));
}

void createSoundEmitter(const std::string& name)
{
    if (state != State::Idle) return;

    // Undo/redo
    std::shared_ptr<NodeContainer> pContainer = std::make_shared<NodeContainer>();
    auto oldSelection = selection;
    actionManager.doAction(new onut::ActionGroup("Create Sound Emitter",
    {
        new onut::Action("",
        [=]{ // OnRedo
            auto pSoundEmitter = pEditingView->CreateSoundEmitter(name);
            pEditingView->AddNode(pSoundEmitter);
            pSoundEmitter->SetPosition(viewSize * .5f);

            auto pTreeItem = new onut::UITreeViewItem();
            pTreeItem->pSharedUserData = pContainer;
            pTreeViewRoot->addItem(pTreeItem);

            pContainer->pNode = pSoundEmitter;
            pContainer->pTreeViewItem = pTreeItem;
            nodesToContainers[pSoundEmitter] = pContainer;
            markModified();
        },
            [=]{ // OnUndo
            auto it = nodesToContainers.find(pContainer->pNode);
            if (it != nodesToContainers.end()) nodesToContainers.erase(it);
            pEditingView->DeleteNode(pContainer->pNode);
            pTreeViewRoot->removeItem(pContainer->pTreeViewItem);
            pContainer->pTreeViewItem = nullptr;
            pContainer->pNode = nullptr;
            markModified();
        },
            [=]{ // Init
        },
            [=]{ // Destroy
        }),
            new onut::Action("",
            [=]{ // OnRedo
            selection.clear();
            selection.push_back(pContainer);
            updateProperties();
        },
            [=]{ // OnUndo
            selection = oldSelection;
            updateProperties();
        }),
    }));
}

void createMusicEmitter(const std::string& name)
{
    if (state != State::Idle) return;

    // Undo/redo
    std::shared_ptr<NodeContainer> pContainer = std::make_shared<NodeContainer>();
    auto oldSelection = selection;
    actionManager.doAction(new onut::ActionGroup("Create Music Emitter",
    {
        new onut::Action("",
        [=]{ // OnRedo
            auto pMusicEmitter = pEditingView->CreateMusicEmitter();
            pMusicEmitter->SetSource(name);
            pEditingView->AddNode(pMusicEmitter);
            pMusicEmitter->SetPosition(viewSize * .5f);

            auto pTreeItem = new onut::UITreeViewItem();
            pTreeItem->pSharedUserData = pContainer;
            pTreeViewRoot->addItem(pTreeItem);

            pContainer->pNode = pMusicEmitter;
            pContainer->pTreeViewItem = pTreeItem;
            nodesToContainers[pMusicEmitter] = pContainer;
            markModified();
        },
            [=]{ // OnUndo
            auto it = nodesToContainers.find(pContainer->pNode);
            if (it != nodesToContainers.end()) nodesToContainers.erase(it);
            pEditingView->DeleteNode(pContainer->pNode);
            pTreeViewRoot->removeItem(pContainer->pTreeViewItem);
            pContainer->pTreeViewItem = nullptr;
            pContainer->pNode = nullptr;
            markModified();
        },
            [=]{ // Init
        },
            [=]{ // Destroy
        }),
            new onut::Action("",
            [=]{ // OnRedo
            selection.clear();
            selection.push_back(pContainer);
            updateProperties();
        },
            [=]{ // OnUndo
            selection = oldSelection;
            updateProperties();
        }),
    }));
}

void createVideo(const std::string& name)
{
    if (state != State::Idle) return;

    // Undo/redo
    std::shared_ptr<NodeContainer> pContainer = std::make_shared<NodeContainer>();
    auto oldSelection = selection;
    actionManager.doAction(new onut::ActionGroup("Create Video",
    {
        new onut::Action("",
        [=]{ // OnRedo
            auto pVideo = pEditingView->CreateVideo();
            pVideo->SetSource(name);
            pEditingView->AddNode(pVideo);
            pVideo->SetPosition(viewSize * .5f);

            auto pTreeItem = new onut::UITreeViewItem();
            pTreeItem->pSharedUserData = pContainer;
            pTreeViewRoot->addItem(pTreeItem);

            pContainer->pNode = pVideo;
            pContainer->pTreeViewItem = pTreeItem;
            nodesToContainers[pVideo] = pContainer;
            markModified();
        },
            [=]{ // OnUndo
            auto it = nodesToContainers.find(pContainer->pNode);
            if (it != nodesToContainers.end()) nodesToContainers.erase(it);
            pEditingView->DeleteNode(pContainer->pNode);
            pTreeViewRoot->removeItem(pContainer->pTreeViewItem);
            pContainer->pTreeViewItem = nullptr;
            pContainer->pNode = nullptr;
            markModified();
        },
            [=]{ // Init
        },
            [=]{ // Destroy
        }),
            new onut::Action("",
            [=]{ // OnRedo
            selection.clear();
            selection.push_back(pContainer);
            updateProperties();
        },
            [=]{ // OnUndo
            selection = oldSelection;
            updateProperties();
        }),
    }));
}

void createTiledMapNode(const std::string& map)
{
    if (state != State::Idle) return;

    // Undo/redo
    std::shared_ptr<NodeContainer> pContainer = std::make_shared<NodeContainer>();
    auto oldSelection = selection;
    actionManager.doAction(new onut::ActionGroup("Create Tiled Map",
    {
        new onut::Action("",
        [=]{ // OnRedo
            auto pTiledMap = pEditingView->CreateTiledMapNode(map);
            pEditingView->AddNode(pTiledMap);

            auto pTreeItem = new onut::UITreeViewItem();
            pTreeItem->pSharedUserData = pContainer;
            pTreeViewRoot->addItem(pTreeItem);

            pContainer->pNode = pTiledMap;
            pContainer->pTreeViewItem = pTreeItem;
            nodesToContainers[pTiledMap] = pContainer;
            markModified();
        },
            [=]{ // OnUndo
            auto it = nodesToContainers.find(pContainer->pNode);
            if (it != nodesToContainers.end()) nodesToContainers.erase(it);
            pEditingView->DeleteNode(pContainer->pNode);
            pTreeViewRoot->removeItem(pContainer->pTreeViewItem);
            pContainer->pTreeViewItem = nullptr;
            pContainer->pNode = nullptr;
            markModified();
        },
            [=]{ // Init
        },
            [=]{ // Destroy
        }),
            new onut::Action("",
            [=]{ // OnRedo
            selection.clear();
            selection.push_back(pContainer);
            updateProperties();
        },
            [=]{ // OnUndo
            selection = oldSelection;
            updateProperties();
        }),
    }));
}

void createSpriteString(const std::string& name)
{
    if (state != State::Idle) return;

    // Undo/redo
    std::shared_ptr<NodeContainer> pContainer = std::make_shared<NodeContainer>();
    auto oldSelection = selection;
    actionManager.doAction(new onut::ActionGroup("Create SpriteString",
    {
        new onut::Action("",
        [=]{ // OnRedo
            auto pSpriteString = pEditingView->CreateSpriteString(name);
            pEditingView->AddNode(pSpriteString);
            pSpriteString->SetPosition(viewSize * .5f);
            pSpriteString->SetCaption("Text");

            auto pTreeItem = new onut::UITreeViewItem();
            pTreeItem->pSharedUserData = pContainer;
            pTreeViewRoot->addItem(pTreeItem);

            pContainer->pNode = pSpriteString;
            pContainer->pTreeViewItem = pTreeItem;
            nodesToContainers[pSpriteString] = pContainer;
            markModified();
        },
            [=]{ // OnUndo
            auto it = nodesToContainers.find(pContainer->pNode);
            if (it != nodesToContainers.end()) nodesToContainers.erase(it);
            pEditingView->DeleteNode(pContainer->pNode);
            pTreeViewRoot->removeItem(pContainer->pTreeViewItem);
            pContainer->pTreeViewItem = nullptr;
            pContainer->pNode = nullptr;
            markModified();
        },
            [=]{ // Init
        },
            [=]{ // Destroy
        }),
            new onut::Action("",
            [=]{ // OnRedo
            selection.clear();
            selection.push_back(pContainer);
            updateProperties();
        },
            [=]{ // OnUndo
            selection = oldSelection;
            updateProperties();
        }),
    }));
}

void startSelectedEmitters()
{
    for (auto pContainer : selection)
    {
        auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
        if (pEmitter)
        {
            pEmitter->Start();
            OSettings->setIsEditorMode(false);
        }
    }
}

void startSelectedSoundEmitters()
{
    for (auto pContainer : selection)
    {
        auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
        if (pSoundEmitter)
        {
            pSoundEmitter->Play();
        }
    }
}

void startSelectedMusicEmitters()
{
    for (auto pContainer : selection)
    {
        auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
        if (pMusicEmitter)
        {
            pMusicEmitter->Play();
        }
    }
}

void startSelectedVideos()
{
    for (auto pContainer : selection)
    {
        auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
        if (pVideo)
        {
            pVideo->Play();
            OSettings->setIsEditorMode(false);
        }
    }
}

// For textbox number spinning
void onStartSpinning(onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
{
    isSpinning = true;
    for (auto pContainer : selection)
    {
        pContainer->stateOnDown = std::make_shared<NodeState>(pContainer);
    }
};

void onStopSpinning(onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
{
    isSpinning = false;
    for (auto pContainer : selection)
    {
        pContainer->stateOnDown->apply();
    }
    pControl->onTextChanged(pControl, event);
};

std::vector<std::shared_ptr<IProp>> props; 

void refreshUI()
{
    if (selection.empty())
    {
        for (auto prop : props)
        {
            prop->updateUI(pEditingView);
        }
    }
    else
    {
        for (auto pContainer : selection)
        {
            auto pNode = pContainer->pNode;
            for (auto prop : props)
            {
                prop->updateUI(pNode);
            }
        }
    }
}

void init()
{
    // Load cursors for different manipulation
    curARROW = LoadCursor(nullptr, IDC_ARROW);
    curSIZENWSE = LoadCursor(nullptr, IDC_SIZENWSE);
    curSIZENESW = LoadCursor(nullptr, IDC_SIZENESW);
    curSIZEWE = LoadCursor(nullptr, IDC_SIZEWE);
    curSIZENS = LoadCursor(nullptr, IDC_SIZENS);
    curSIZEALL = LoadCursor(nullptr, IDC_SIZEALL);

    // Create the UI styles and load our editor UI
    createUIStyles(OUIContext);
    OUI->add(OLoadUI("editor.json"));

    // Get/Setup basic editor UI stuff
    ui_mainView = OFindUI("mainView");
    ui_treeView = dynamic_cast<onut::UITreeView*>(OFindUI("treeView"));
    ui_treeView->allowReorder = true;
    ui_btnCreateNode = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateNode"));
    ui_btnCreateSprite = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateSprite"));
    ui_btnCreateSpriteString = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateSpriteString"));
    ui_btnCreateEmitter = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateEmitter"));
    ui_btnCreateSoundEmitter = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateSoundEmitter"));
    ui_btnCreateMusicEmitter = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateMusicEmitter"));
    ui_btnCreateVideo = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateVideo"));
    ui_btnCreateEffect = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateEffect"));
    ui_btnCreateTiledMap = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateTiledMap"));

    // Register properties panel for their respective type
    registerPropertiesPanel<void>("propertiesView");
    registerPropertiesPanel<seed::Node>("propertiesNode");
    registerPropertiesPanel<seed::Sprite>("propertiesNode");
    registerPropertiesPanel<seed::Sprite>("propertiesSprite");
    registerPropertiesPanel<seed::SpriteString>("propertiesNode");
    registerPropertiesPanel<seed::SpriteString>("propertiesSprite");
    registerPropertiesPanel<seed::SpriteString>("propertiesSpriteString");
    registerPropertiesPanel<seed::Emitter>("propertiesNode");
    registerPropertiesPanel<seed::Emitter>("propertiesEmitter");
    registerPropertiesPanel<seed::SoundEmitter>("propertiesNode");
    registerPropertiesPanel<seed::SoundEmitter>("propertiesSoundEmitter");
    registerPropertiesPanel<seed::MusicEmitter>("propertiesNode");
    registerPropertiesPanel<seed::MusicEmitter>("propertiesMusicEmitter");
    registerPropertiesPanel<seed::Video>("propertiesNode");
    registerPropertiesPanel<seed::Video>("propertiesVideo");
    registerPropertiesPanel<seed::Effect>("propertiesNode");
    registerPropertiesPanel<seed::Effect>("propertiesEffect");
    registerPropertiesPanel<seed::TiledMapNode>("propertiesNode");
    registerPropertiesPanel<seed::TiledMapNode>("propertiesTiledMap");

    // Register properties
    // View
    registerVector2Property<seed::View>("View Dimension", "txtViewWidth", "txtViewHeight", &seed::View::GetSize, &seed::View::SetSize);

    // Node
    registerStringProperty<seed::Node>("Name", "txtSpriteName", &seed::Node::GetName, &seed::Node::SetName);
    registerBoolProperty<seed::Node>("Visible", "chkNodeVisible", &seed::Node::GetVisible, &seed::Node::SetVisible);
    registerVector2Property<seed::Node>("Position", "txtSpriteX", "txtSpriteY", &seed::Node::GetPosition, &seed::Node::SetPosition);
    registerVector2Property<seed::Node>("Scale", "txtSpriteScaleX", "txtSpriteScaleY", &seed::Node::GetScale, &seed::Node::SetScale, {.01f}, {.01f});
    registerFloatProperty<seed::Node>("Angle", "txtSpriteAngle", &seed::Node::GetAngle, &seed::Node::SetAngle);
    registerColorProperty<seed::Node>("Color", "colSpriteColor", "txtSpriteAlpha", &seed::Node::GetColor, &seed::Node::SetColor);

    // Sprite
    ui_txtSpriteTexture = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteTexture"));
    ui_btnSpriteTextureBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnSpriteTextureBrowse"));
    registerBoolProperty<seed::Sprite>("Flipped H", "chkSpriteFlippedH", &seed::Sprite::GetFlippedH, &seed::Sprite::SetFlippedH);
    registerBoolProperty<seed::Sprite>("Flipped V", "chkSpriteFlippedV", &seed::Sprite::GetFlippedV, &seed::Sprite::SetFlippedV);
    ui_chkSpriteBlends[0] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSpriteBlendAdd"));
    ui_chkSpriteBlends[1] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSpriteBlendAlpha"));
    ui_chkSpriteBlends[2] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSpriteBlendPreMult"));
    ui_chkSpriteBlends[3] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSpriteBlendMult"));
    for (auto pRadio : ui_chkSpriteBlends)
    {
        pRadio->behavior = onut::eUICheckBehavior::EXCLUSIVE;
    }
    ui_chkSpriteFilters[0] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSpriteFilterNearest"));
    ui_chkSpriteFilters[1] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSpriteFilterLinear"));
    for (auto pRadio : ui_chkSpriteFilters)
    {
        pRadio->behavior = onut::eUICheckBehavior::EXCLUSIVE;
    }
    registerVector2Property<seed::Sprite>("Align", "txtSpriteAlignX", "txtSpriteAlignY", &seed::Sprite::GetAlign, &seed::Sprite::SetAlign, {.01f}, {.01f});

    // SpriteString
    ui_txtSpriteStringFont = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteStringFont"));
    ui_btnSpriteStringFontBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnSpriteStringFontBrowse"));
    registerStringProperty<seed::SpriteString>("Caption", "txtSpriteStringCaption", &seed::SpriteString::GetCaption, &seed::SpriteString::SetCaption);

    // Emitter
    ui_txtEmitterFx = dynamic_cast<onut::UITextBox*>(OFindUI("txtEmitterFx"));
    ui_txtEmitterFxBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnEmitterFxBrowse"));
    registerBoolProperty<seed::Emitter>("Emit World", "chkEmitterEmitWorld", &seed::Emitter::GetEmitWorld, &seed::Emitter::SetEmitWorld);
    ui_chkEmitterBlends[0] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkEmitterBlendAdd"));
    ui_chkEmitterBlends[1] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkEmitterBlendAlpha"));
    ui_chkEmitterBlends[2] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkEmitterBlendPreMult"));
    ui_chkEmitterBlends[3] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkEmitterBlendMult"));
    for (auto pRadio : ui_chkEmitterBlends)
    {
        pRadio->behavior = onut::eUICheckBehavior::EXCLUSIVE;
    }
    ui_chkEmitterFilters[0] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkEmitterFilterNearest"));
    ui_chkEmitterFilters[1] = dynamic_cast<onut::UICheckBox*>(OFindUI("chkEmitterFilterLinear"));
    for (auto pRadio : ui_chkEmitterFilters)
    {
        pRadio->behavior = onut::eUICheckBehavior::EXCLUSIVE;
    }

    // SoundEmitter
    ui_txtSoundEmitter = dynamic_cast<onut::UITextBox*>(OFindUI("txtSoundEmitter"));
    ui_btnSoundEmitterBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnSoundEmitterBrowse"));
    registerBoolProperty<seed::SoundEmitter>("Loop", "chkSoundEmitterLoop", &seed::SoundEmitter::GetLoops, &seed::SoundEmitter::SetLoops);
    registerBoolProperty<seed::SoundEmitter>("Position Based", "chkSoundEmitterPositionBased", &seed::SoundEmitter::GetPositionBased, &seed::SoundEmitter::SetPositionBased);
    registerFloatProperty<seed::SoundEmitter>("Volume", "txtSoundEmitterVolume", &seed::SoundEmitter::GetVolume, &seed::SoundEmitter::SetVolume, {1.f, 0.f, 100.f}, .01f);
    registerFloatProperty<seed::SoundEmitter>("Balance", "txtSoundEmitterBalance", &seed::SoundEmitter::GetBalance, &seed::SoundEmitter::SetBalance, {1.f, -100.f, 100.f}, .01f);
    registerFloatProperty<seed::SoundEmitter>("Pitch", "txtSoundEmitterPitch", &seed::SoundEmitter::GetBalance, &seed::SoundEmitter::SetBalance, {1.f, 10.f, 200.f}, .01f);

    // MusicEmitter
    ui_txtMusicEmitter = dynamic_cast<onut::UITextBox*>(OFindUI("txtMusicEmitter"));
    ui_btnMusicEmitterBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnMusicEmitterBrowse"));
    registerBoolProperty<seed::MusicEmitter>("Loop", "chkMusicEmitterLoop", &seed::MusicEmitter::GetLoops, &seed::MusicEmitter::SetLoops);
    registerFloatProperty<seed::MusicEmitter>("Volume", "txtMusicEmitterVolume", &seed::MusicEmitter::GetVolume, &seed::MusicEmitter::SetVolume, {1.f, 0.f, 100.f}, .01f);

    // Video
    ui_txtVideo = dynamic_cast<onut::UITextBox*>(OFindUI("txtVideo"));
    ui_btnVideoBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnVideoBrowse"));
    registerBoolProperty<seed::Video>("Loop", "chkVideoLoop", &seed::Video::GetLoops, &seed::Video::SetLoops);
    registerFloatProperty<seed::Video>("Volume", "txtVideoVolume", &seed::Video::GetVolume, &seed::Video::SetVolume, {1.f, 0.f, 100.f}, .01f);
    registerDoubleProperty<seed::Video>("Pitch", "txtVideoPlayRate", &seed::Video::GetPlayRate, &seed::Video::SetPlayRate, {1.f, 10.f, 200.f}, .01f);
    registerVector2Property<seed::Video>("Position", "txtVideoWidth", "txtVideoHeight", &seed::Video::GetDimensions, &seed::Video::SetDimensions);

    // Effect
    registerBoolProperty<seed::Effect>("Blur", "chkEffectBlurEnabled", &seed::Effect::GetBlurEnabled, &seed::Effect::SetBlurEnabled);
    registerFloatProperty<seed::Effect>("Blur Amount", "txtEffectBlurAmount", &seed::Effect::GetBlurAmount, &seed::Effect::SetBlurAmount, {.1f, 0.f, 1024.f});
    registerBoolProperty<seed::Effect>("Sepia", "chkEffectSepiaEnabled", &seed::Effect::GetSepiaEnabled, &seed::Effect::SetSepiaEnabled);
    registerVector3Property<seed::Effect>("Sepia Tone", "txtEffectSepiaToneR", "txtEffectSepiaToneG", "txtEffectSepiaToneB", &seed::Effect::GetSepiaTone, &seed::Effect::SetSepiaTone, {.01f, 0.f, 2.55f}, {.01f, 0.f, 2.55f}, {.01f, 0.f, 2.55f});
    registerFloatProperty<seed::Effect>("Sepia Saturation", "txtEffectSepiaSaturation", &seed::Effect::GetSepiaSaturation, &seed::Effect::SetSepiaSaturation, {1.f, 0.f, 100.f}, .01f);
    registerFloatProperty<seed::Effect>("Sepia Amount", "txtEffectSepiaAmount", &seed::Effect::GetSepiaAmount, &seed::Effect::SetSepiaAmount, {1.f, 0.f, 100.f}, .01f);
    registerBoolProperty<seed::Effect>("Crt", "chkEffectCrtEnabled", &seed::Effect::GetCrtEnabled, &seed::Effect::SetCrtEnabled);
    registerBoolProperty<seed::Effect>("Cartoon", "chkEffectCartoonEnabled", &seed::Effect::GetCartoonEnabled, &seed::Effect::SetCartoonEnabled);
    registerVector3Property<seed::Effect>("Cartoon Tone", "txtEffectCartoonToneR", "txtEffectCartoonToneG", "txtEffectCartoonToneB", &seed::Effect::GetCartoonTone, &seed::Effect::SetCartoonTone, {.1f, 1.f}, {.1f, 1.f}, {.1f, 1.f});
    registerBoolProperty<seed::Effect>("Vignette", "chkEffectVignetteEnabled", &seed::Effect::GetVignetteEnabled, &seed::Effect::SetVignetteEnabled);
    registerFloatProperty<seed::Effect>("Vignette Amount", "txtEffectVignetteAmount", &seed::Effect::GetVignetteAmount, &seed::Effect::SetVignetteAmount, {1.f, 0.f, 100.f}, .01f);

    // TiledMapNode
    ui_txtTiledMap = dynamic_cast<onut::UITextBox*>(OFindUI("txtTiledMap"));
    ui_btnTiledMapBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnTiledMapBrowse"));

    ui_treeView->onMoveItemInto = [](onut::UITreeView* in_pTreeView, const onut::UITreeViewMoveEvent& event)
    {
        auto pTargetContainer = std::static_pointer_cast<NodeContainer>(event.pTarget->pSharedUserData);
        if (pTargetContainer)
        {
            auto pGroup = new onut::ActionGroup("Parent");
            for (auto pItem : *event.pSelectedItems)
            {
                auto pContainer = std::static_pointer_cast<NodeContainer>(pItem->pSharedUserData);
                auto pParentContainer = nodesToContainers[pContainer->pNode->GetParent()];
                std::shared_ptr<NodeContainer> pAfterSibbling = nullptr;
                auto& bgChildren = pParentContainer->pNode->GetBgChildren();
                auto& fgChildren = pParentContainer->pNode->GetFgChildren();
                for (decltype(bgChildren.size()) i = 0; i < bgChildren.size(); ++i)
                {
                    if (bgChildren[i] == pContainer->pNode)
                    {
                        if (i < bgChildren.size() - 1)
                        {
                            pAfterSibbling = nodesToContainers[bgChildren[i + 1]];
                            break;
                        }
                    }
                }
                if (!pAfterSibbling)
                {
                    for (decltype(fgChildren.size()) i = 0; i < fgChildren.size(); ++i)
                    {
                        if (fgChildren[i] == pContainer->pNode)
                        {
                            if (i < fgChildren.size() - 1)
                            {
                                pAfterSibbling = nodesToContainers[fgChildren[i + 1]];
                                break;
                            }
                        }
                    }
                }

                auto stateBefore = std::make_shared<NodeState>(pContainer);
                transformToParent(pTargetContainer->pNode, pContainer->pNode);
                auto stateAfter = std::make_shared<NodeState>(pContainer);

                pGroup->addAction(new onut::Action("",
                    [=]
                {
                    pContainer->pNode->GetParent()->Detach(pContainer->pNode);
                    pTargetContainer->pNode->Attach(pContainer->pNode);
                    stateAfter->apply();
                    updateProperties();
                    pTargetContainer->pTreeViewItem->addItem(pContainer->pTreeViewItem);
                    markModified();
                }, [=]
                {
                    pContainer->pNode->GetParent()->Detach(pContainer->pNode);
                    if (pAfterSibbling)
                    {
                        pParentContainer->pNode->AttachBefore(pContainer->pNode, pAfterSibbling->pNode);
                        pParentContainer->pTreeViewItem->addItemBefore(pContainer->pTreeViewItem, pAfterSibbling->pTreeViewItem);
                    }
                    else
                    {
                        pParentContainer->pNode->Attach(pContainer->pNode);
                        pParentContainer->pTreeViewItem->addItem(pContainer->pTreeViewItem);
                    }
                    stateBefore->apply();
                    updateProperties();
                    markModified();
                }));
            }
            actionManager.doAction(pGroup);
        }
    };

    ui_treeView->onMoveItemBefore = [](onut::UITreeView* in_pTreeView, const onut::UITreeViewMoveEvent& event)
    {
        auto pTargetContainer = std::static_pointer_cast<NodeContainer>(event.pTarget->pSharedUserData);
        if (pTargetContainer)
        {
            auto pGroup = new onut::ActionGroup("Reorder");

            auto pTargetParent = pTargetContainer->pNode->GetParent();
            for (auto pItem : *event.pSelectedItems)
            {
                auto pContainer = std::static_pointer_cast<NodeContainer>(pItem->pSharedUserData);
                auto pParentContainer = nodesToContainers[pContainer->pNode->GetParent()];
                std::shared_ptr<NodeContainer> pAfterSibbling = nullptr;
                auto& bgChildren = pParentContainer->pNode->GetBgChildren();
                auto& fgChildren = pParentContainer->pNode->GetFgChildren();
                for (decltype(bgChildren.size()) i = 0; i < bgChildren.size(); ++i)
                {
                    if (bgChildren[i] == pContainer->pNode)
                    {
                        if (i < bgChildren.size() - 1)
                        {
                            pAfterSibbling = nodesToContainers[bgChildren[i + 1]];
                            break;
                        }
                    }
                }
                if (!pAfterSibbling)
                {
                    for (decltype(fgChildren.size()) i = 0; i < fgChildren.size(); ++i)
                    {
                        if (fgChildren[i] == pContainer->pNode)
                        {
                            if (i < fgChildren.size() - 1)
                            {
                                pAfterSibbling = nodesToContainers[fgChildren[i + 1]];
                                break;
                            }
                        }
                    }
                }
                auto pTargetParentContainer = nodesToContainers[pTargetParent];

                auto stateBefore = std::make_shared<NodeState>(pContainer);
                transformToParent(pTargetParent, pContainer->pNode);
                auto stateAfter = std::make_shared<NodeState>(pContainer);

                pGroup->addAction(new onut::Action("",
                    [=]
                {
                    pContainer->pNode->GetParent()->Detach(pContainer->pNode);
                    pTargetParentContainer->pNode->AttachBefore(pContainer->pNode, pTargetContainer->pNode);
                    stateAfter->apply();
                    updateProperties();
                    pTargetParentContainer->pTreeViewItem->addItemBefore(pContainer->pTreeViewItem, pTargetContainer->pTreeViewItem);
                    markModified();
                }, [=]
                {
                    pContainer->pNode->GetParent()->Detach(pContainer->pNode);
                    if (pAfterSibbling)
                    {
                        pParentContainer->pNode->AttachBefore(pContainer->pNode, pAfterSibbling->pNode);
                        pParentContainer->pTreeViewItem->addItemBefore(pContainer->pTreeViewItem, pAfterSibbling->pTreeViewItem);
                    }
                    else
                    {
                        pParentContainer->pNode->Attach(pContainer->pNode);
                        pParentContainer->pTreeViewItem->addItem(pContainer->pTreeViewItem);
                    }
                    stateBefore->apply();
                    updateProperties();
                    markModified();
                }));
            }

            actionManager.doAction(pGroup);
        }
    };

    ui_treeView->onMoveItemAfter = [](onut::UITreeView* in_pTreeView, const onut::UITreeViewMoveEvent& event)
    {
        auto pTargetContainer = std::static_pointer_cast<NodeContainer>(event.pTarget->pSharedUserData);
        if (pTargetContainer)
        {
            auto pGroup = new onut::ActionGroup("Reorder");

            auto pTargetParent = pTargetContainer->pNode->GetParent();
            for (auto pItem : *event.pSelectedItems)
            {
                auto pContainer = std::static_pointer_cast<NodeContainer>(pItem->pSharedUserData);
                auto pParentContainer = nodesToContainers[pContainer->pNode->GetParent()];
                std::shared_ptr<NodeContainer> pAfterSibbling = nullptr;
                auto& bgChildren = pParentContainer->pNode->GetBgChildren();
                auto& fgChildren = pParentContainer->pNode->GetFgChildren();
                for (decltype(bgChildren.size()) i = 0; i < bgChildren.size(); ++i)
                {
                    if (bgChildren[i] == pContainer->pNode)
                    {
                        if (i < bgChildren.size() - 1)
                        {
                            pAfterSibbling = nodesToContainers[bgChildren[i + 1]];
                            break;
                        }
                    }
                }
                if (!pAfterSibbling)
                {
                    for (decltype(fgChildren.size()) i = 0; i < fgChildren.size(); ++i)
                    {
                        if (fgChildren[i] == pContainer->pNode)
                        {
                            if (i < fgChildren.size() - 1)
                            {
                                pAfterSibbling = nodesToContainers[fgChildren[i + 1]];
                                break;
                            }
                        }
                    }
                }
                auto pTargetParentContainer = nodesToContainers[pTargetParent];

                auto stateBefore = std::make_shared<NodeState>(pContainer);
                transformToParent(pTargetParent, pContainer->pNode);
                auto stateAfter = std::make_shared<NodeState>(pContainer);

                pGroup->addAction(new onut::Action("",
                    [=]
                {
                    pContainer->pNode->GetParent()->Detach(pContainer->pNode);
                    pTargetParentContainer->pNode->AttachAfter(pContainer->pNode, pTargetContainer->pNode);
                    stateAfter->apply();
                    updateProperties();
                    pTargetParentContainer->pTreeViewItem->addItemAfter(pContainer->pTreeViewItem, pTargetContainer->pTreeViewItem);
                    markModified();
                }, [=]
                {
                    pContainer->pNode->GetParent()->Detach(pContainer->pNode);
                    if (pAfterSibbling)
                    {
                        pParentContainer->pNode->AttachBefore(pContainer->pNode, pAfterSibbling->pNode);
                        pParentContainer->pTreeViewItem->addItemBefore(pContainer->pTreeViewItem, pAfterSibbling->pTreeViewItem);
                    }
                    else
                    {
                        pParentContainer->pNode->Attach(pContainer->pNode);
                        pParentContainer->pTreeViewItem->addItem(pContainer->pTreeViewItem);
                    }
                    stateBefore->apply();
                    updateProperties();
                    markModified();
                }));
            }

            actionManager.doAction(pGroup);
        }
    };

    ui_txtEmitterFx->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Fx", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            if (pEmitter)
            {
                bool restart = pEmitter->GetEmitterInstance().isPlaying();
                pEmitter->Init(ui_txtEmitterFx->textComponent.text);
                if (restart)
                {
                    pEmitter->Stop();
                    pEmitter->Start();
                }
            }
        });
    };
    ui_txtEmitterFxBrowse->onClick = [=](onut::UIControl* pControl, const onut::UIMouseEvent& mouseEvent)
    {
        std::string file = fileOpen("Onut Fx (*.pfx)\0*.pfx\0Designer (*.pex)\0*.pex\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            // Make it relative to our filename
            ui_txtEmitterFx->textComponent.text = onut::getFilename(file);
            if (ui_txtEmitterFx->onTextChanged)
            {
                onut::UITextBoxEvent evt;
                evt.pContext = OUIContext;
                ui_txtEmitterFx->onTextChanged(ui_txtEmitterFx, evt);
            }
        }
    };
    ui_txtSpriteTexture->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Texture", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetTexture(OGetTexture(ui_txtSpriteTexture->textComponent.text.c_str()));
            }
        });
    };
    ui_btnSpriteTextureBrowse->onClick = [=](onut::UIControl* pControl, const onut::UIMouseEvent& mouseEvent)
    {
        std::string file = fileOpen("Image Files (*.png)\0*.png\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            // Make it relative to our filename
            ui_txtSpriteTexture->textComponent.text = onut::getFilename(file);
            if (ui_txtSpriteTexture->onTextChanged)
            {
                onut::UITextBoxEvent evt;
                evt.pContext = OUIContext;
                ui_txtSpriteTexture->onTextChanged(ui_txtSpriteTexture, evt);
            }
        }
    };
    ui_txtSoundEmitter->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Sound", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
            if (pSoundEmitter)
            {
                pSoundEmitter->Init(ui_txtSoundEmitter->textComponent.text);
            }
        });
    };
    ui_btnSoundEmitterBrowse->onClick = [=](onut::UIControl* pControl, const onut::UIMouseEvent& mouseEvent)
    {
        std::string file = fileOpen("Wav Files (*.wav)\0*.wav\0Cue Files (*.cue)\0*.cue\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            // Make it relative to our filename
            ui_txtSoundEmitter->textComponent.text = onut::getFilename(file);
            if (ui_txtSoundEmitter->onTextChanged)
            {
                onut::UITextBoxEvent evt;
                evt.pContext = OUIContext;
                ui_txtSoundEmitter->onTextChanged(ui_txtSoundEmitter, evt);
            }
        }
    };
    ui_txtMusicEmitter->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Music", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
            if (pMusicEmitter)
            {
                pMusicEmitter->SetSource(ui_txtMusicEmitter->textComponent.text);
            }
        });
    };
    ui_btnMusicEmitterBrowse->onClick = [=](onut::UIControl* pControl, const onut::UIMouseEvent& mouseEvent)
    {
        std::string file = fileOpen("MP3 Files (*.mp3)\0*.mp3\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            // Make it relative to our filename
            ui_txtMusicEmitter->textComponent.text = onut::getFilename(file);
            if (ui_txtMusicEmitter->onTextChanged)
            {
                onut::UITextBoxEvent evt;
                evt.pContext = OUIContext;
                ui_txtMusicEmitter->onTextChanged(ui_txtMusicEmitter, evt);
            }
        }
    };
    ui_txtVideo->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Video", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
            if (pVideo)
            {
                pVideo->SetSource(ui_txtVideo->textComponent.text);
            }
        });
    };
    ui_btnVideoBrowse->onClick = [=](onut::UIControl* pControl, const onut::UIMouseEvent& mouseEvent)
    {
        std::string file = fileOpen("MP4 Files (*.mp4)\0*.mp4\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            // Make it relative to our filename
            ui_txtVideo->textComponent.text = onut::getFilename(file);
            if (ui_txtVideo->onTextChanged)
            {
                onut::UITextBoxEvent evt;
                evt.pContext = OUIContext;
                ui_txtVideo->onTextChanged(ui_txtVideo, evt);
            }
        }
    };
    ui_txtSpriteStringFont->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Font", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSpriteString = dynamic_cast<seed::SpriteString*>(pContainer->pNode);
            if (pSpriteString)
            {
                pSpriteString->SetFont(OGetBMFont(ui_txtSpriteStringFont->textComponent.text.c_str()));
            }
        });
    };
    ui_btnSpriteStringFontBrowse->onClick = [=](onut::UIControl* pControl, const onut::UIMouseEvent& mouseEvent)
    {
        std::string file = fileOpen("BMFont Files (*.fnt)\0*.fnt\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            // Make it relative to our filename
            ui_txtSpriteStringFont->textComponent.text = onut::getFilename(file);
            if (ui_txtSpriteStringFont->onTextChanged)
            {
                onut::UITextBoxEvent evt;
                evt.pContext = OUIContext;
                ui_txtSpriteStringFont->onTextChanged(ui_txtSpriteStringFont, evt);
            }
        }
    };
    ui_txtTiledMap->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Tiled Map", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pTiledMapNode = dynamic_cast<seed::TiledMapNode*>(pContainer->pNode);
            if (pTiledMapNode)
            {
                pTiledMapNode->Init(ui_txtTiledMap->textComponent.text);
            }
        });
    };
    ui_btnTiledMapBrowse->onClick = [=](onut::UIControl* pControl, const onut::UIMouseEvent& mouseEvent)
    {
        std::string file = fileOpen("TMX Files (*.tmx)\0*.tmx\0All Files (*.*)\0*.*\0");
        if (!file.empty())
        {
            // Make it relative to our filename
            ui_txtTiledMap->textComponent.text = onut::getFilename(file);
            if (ui_txtTiledMap->onTextChanged)
            {
                onut::UITextBoxEvent evt;
                evt.pContext = OUIContext;
                ui_txtTiledMap->onTextChanged(ui_txtTiledMap, evt);
            }
        }
    };
    ui_chkSpriteBlends[0]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Blend", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetBlend(onut::SpriteBatch::eBlendMode::Add);
            }
        });
    };
    ui_chkSpriteBlends[1]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Blend", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetBlend(onut::SpriteBatch::eBlendMode::Alpha);
            }
        });
    };
    ui_chkSpriteBlends[2]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Blend", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetBlend(onut::SpriteBatch::eBlendMode::PreMultiplied);
            }
        });
    };
    ui_chkSpriteBlends[3]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Blend", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetBlend(onut::SpriteBatch::eBlendMode::Multiplied);
            }
        });
    };
    ui_chkSpriteFilters[0]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Filter", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetFilter(onut::SpriteBatch::eFiltering::Nearest);
            }
        });
    };
    ui_chkSpriteFilters[1]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Filter", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetFilter(onut::SpriteBatch::eFiltering::Linear);
            }
        });
    };
    ui_chkEmitterBlends[0]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Blend", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            if (pEmitter)
            {
                pEmitter->SetBlend(onut::SpriteBatch::eBlendMode::Add);
            }
        });
    };
    ui_chkEmitterBlends[1]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Blend", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            if (pEmitter)
            {
                pEmitter->SetBlend(onut::SpriteBatch::eBlendMode::Alpha);
            }
        });
    };
    ui_chkEmitterBlends[2]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Blend", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            if (pEmitter)
            {
                pEmitter->SetBlend(onut::SpriteBatch::eBlendMode::PreMultiplied);
            }
        });
    };
    ui_chkEmitterBlends[3]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Blend", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            if (pEmitter)
            {
                pEmitter->SetBlend(onut::SpriteBatch::eBlendMode::Multiplied);
            }
        });
    };
    ui_chkEmitterFilters[0]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Filter", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            if (pEmitter)
            {
                pEmitter->SetFilter(onut::SpriteBatch::eFiltering::Nearest);
            }
        });
    };
    ui_chkEmitterFilters[1]->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        if (!pControl->getIsChecked()) return;
        changeSpriteProperty("Change Filter", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            if (pEmitter)
            {
                pEmitter->SetFilter(onut::SpriteBatch::eFiltering::Linear);
            }
        });
    };

    dottedLineAnim.start(0.f, -1.f, .5f, OLinear, OLoop);

    buildMenu();
    OWindow->onMenu = onMenu;
    OWindow->onWrite = [](char c){OUIContext->write(c); };
    OWindow->onKey = [](uintptr_t key)
    {
        if (state != State::Idle) return;
        OUIContext->keyDown(key);
        if (!dynamic_cast<onut::UITextBox*>(OUIContext->getFocusControl()))
        {
            checkNudge(key);
            checkShortCut(key);
        }
    };

    auto mainViewRect = onut::UI2Onut(ui_mainView->getWorldRect(*OUIContext));
    cameraPos.x = viewSize.x * .5f;
    cameraPos.y = viewSize.y * .5f;

    // Bind toolbox actions
    ui_btnCreateSprite->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        createSprite("default.png");
    };

    ui_btnCreateEmitter->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        createEmitter("");
    };

    ui_btnCreateSoundEmitter->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        createSoundEmitter("");
    };

    ui_btnCreateMusicEmitter->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        createMusicEmitter("");
    };

    ui_btnCreateVideo->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        createVideo("");
    };

    ui_btnCreateTiledMap->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        createTiledMapNode("");
    };

    ui_btnCreateSpriteString->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        createSpriteString("segeo12.fnt");
    };

    ui_btnCreateEffect->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        if (state != State::Idle) return;

        // Undo/redo
        std::shared_ptr<NodeContainer> pContainer = std::make_shared<NodeContainer>();
        auto oldSelection = selection;
        actionManager.doAction(new onut::ActionGroup("Create Effect",
        {
            new onut::Action("",
            [=]{ // OnRedo
                auto pNode = pEditingView->CreateEffect();
                pEditingView->AddNode(pNode);
                pNode->SetPosition(viewSize * .5f);

                auto pTreeItem = new onut::UITreeViewItem();
                pTreeItem->pSharedUserData = pContainer;
                pTreeViewRoot->addItem(pTreeItem);

                pContainer->pNode = pNode;
                pContainer->pTreeViewItem = pTreeItem;
                nodesToContainers[pNode] = pContainer;
                markModified();
            },
                [=]{ // OnUndo
                auto it = nodesToContainers.find(pContainer->pNode);
                if (it != nodesToContainers.end()) nodesToContainers.erase(it);
                pEditingView->DeleteNode(pContainer->pNode);
                pTreeViewRoot->removeItem(pContainer->pTreeViewItem);
                pContainer->pTreeViewItem = nullptr;
                pContainer->pNode = nullptr;
                markModified();
            },
                [=]{ // Init
            },
                [=]{ // Destroy
            }),
                new onut::Action("",
                [=]{ // OnRedo
                selection.clear();
                selection.push_back(pContainer);
                updateProperties();
            },
                [=]{ // OnUndo
                selection = oldSelection;
                updateProperties();
            }),
        }));
    };

    ui_btnCreateNode->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        if (state != State::Idle) return;

        // Undo/redo
        std::shared_ptr<NodeContainer> pContainer = std::make_shared<NodeContainer>();
        auto oldSelection = selection;
        actionManager.doAction(new onut::ActionGroup("Create Node",
        {
            new onut::Action("",
                [=]{ // OnRedo
                auto pNode = pEditingView->CreateNode();
                pEditingView->AddNode(pNode);
                pNode->SetPosition(viewSize * .5f);

                auto pTreeItem = new onut::UITreeViewItem();
                pTreeItem->pSharedUserData = pContainer;
                pTreeViewRoot->addItem(pTreeItem);

                pContainer->pNode = pNode;
                pContainer->pTreeViewItem = pTreeItem;
                nodesToContainers[pNode] = pContainer;
                markModified();
            },
                [=]{ // OnUndo
                auto it = nodesToContainers.find(pContainer->pNode);
                if (it != nodesToContainers.end()) nodesToContainers.erase(it);
                pEditingView->DeleteNode(pContainer->pNode);
                pTreeViewRoot->removeItem(pContainer->pTreeViewItem);
                pContainer->pTreeViewItem = nullptr;
                pContainer->pNode = nullptr;
                markModified();
            },
                [=]{ // Init
            },
                [=]{ // Destroy
            }),
            new onut::Action("",
                [=]{ // OnRedo
                selection.clear();
                selection.push_back(pContainer);
                updateProperties();
            },
                [=]{ // OnUndo
                selection = oldSelection;
                updateProperties();
            }),
        }));
    };

    // Camera panning
    ui_mainView->onMiddleMouseDown = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        if (state != State::Idle) return;
        state = State::Panning;
        cameraPosOnDown = cameraPos;
        mousePosOnDown = event.mousePos;
    };
    ui_mainView->onMiddleMouseUp = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        if (state != State::Panning) return;
        state = State::Idle;
    };

    // Draw the seed::View
    OUIContext->addStyle<onut::UIPanel>("mainView", [](const onut::UIControl* pControl, const onut::sUIRect& rect)
    {
        OSB->drawRect(nullptr, onut::UI2Onut(rect), OColorHex(232323));
        OSB->end();

        Matrix transform = getViewTransform();
        OSB->begin(transform);
        OSB->drawRect(nullptr, Rect(0, 0, viewSize.x, viewSize.y), Color::Black);
        if (pEditingView)
        {
            pEditingView->Render();
            // Gizmos
            pEditingView->VisitNodes([&](seed::Node *pNode) -> bool
            {
                auto pSprite = dynamic_cast<seed::Sprite*>(pNode);
                auto pEmitter = dynamic_cast<seed::Emitter*>(pNode);
                auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pNode);
                auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pNode);
                auto pVideo = dynamic_cast<seed::Video*>(pNode);
                auto pEffect = dynamic_cast<seed::Effect*>(pNode);
                if (pSprite) {}
                else if (pEmitter)
                {
                    OSB->drawSprite(OGetTexture("icoEmitter.png"), pNode->GetTransform().Translation());
                }
                else if (pSoundEmitter)
                {
                    OSB->drawSprite(OGetTexture("sound.png"), pNode->GetTransform().Translation());
                }
                else if (pMusicEmitter)
                {
                    OSB->drawSprite(OGetTexture("music.png"), pNode->GetTransform().Translation());
                }
                else if (pVideo)
                {
                    OSB->drawSprite(OGetTexture("video.png"), pNode->GetTransform().Translation());
                }
                else if (pEffect)
                {
                    OSB->drawSprite(OGetTexture("effect.png"), pNode->GetTransform().Translation());
                }
                return false;
            });
        }

        // Draw selection
        const Color DOTTED_LINE_COLOR = {1, 1, 1, .5f};
        const Color AABB_DOTTED_LINE_COLOR = {1, 1, .5f, .5f};
        const Color HANDLE_COLOR = OColorHex(999999);

        auto pDottedLineTexture = OGetTexture("dottedLine.png");
        auto dottedLineScale = 1.f / pDottedLineTexture->getSizef().x * zoom;
        auto dottedLineOffset = dottedLineAnim.get();
        OSB->end();

        for (auto pContainer : selection)
        {
            auto pNode = pContainer->pNode;
            auto pSprite = dynamic_cast<seed::Sprite*>(pNode);
            auto pVideo = dynamic_cast<seed::Video*>(pNode);

            if (pSprite)
            {
                auto points = getSpriteCorners(pSprite);
                Vector2 size = {pSprite->GetWidth(), pSprite->GetHeight()};

                OPB->begin(onut::ePrimitiveType::LINE_STRIP, pDottedLineTexture);
                OPB->draw(points[0], DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset));
                OPB->draw(points[1], DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset + size.y * dottedLineScale * pSprite->GetScale().y));
                OPB->draw(points[2], DOTTED_LINE_COLOR, Vector2(dottedLineOffset + size.x * dottedLineScale * pSprite->GetScale().x, dottedLineOffset + size.y * dottedLineScale * pSprite->GetScale().y));
                OPB->draw(points[3], DOTTED_LINE_COLOR, Vector2(dottedLineOffset + size.x * dottedLineScale * pSprite->GetScale().x, dottedLineOffset));
                OPB->draw(points[0], DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset));
                OPB->end();

                if (!isMultiSelection())
                {
                    auto spriteTransform = pSprite->GetTransform();
                    OSB->begin(transform);
                    OSB->drawCross(spriteTransform.Translation(), 10.f, HANDLE_COLOR);
                    OSB->end();
                }
            }
            else if (pVideo)
            {
                auto points = getVideoCorners(pVideo);
                Vector2 size = {pVideo->GetWidth(), pVideo->GetHeight()};

                OPB->begin(onut::ePrimitiveType::LINE_STRIP, pDottedLineTexture);
                OPB->draw(points[0], DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset));
                OPB->draw(points[1], DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset + size.y * dottedLineScale * pVideo->GetScale().y));
                OPB->draw(points[2], DOTTED_LINE_COLOR, Vector2(dottedLineOffset + size.x * dottedLineScale * pVideo->GetScale().x, dottedLineOffset + size.y * dottedLineScale * pVideo->GetScale().y));
                OPB->draw(points[3], DOTTED_LINE_COLOR, Vector2(dottedLineOffset + size.x * dottedLineScale * pVideo->GetScale().x, dottedLineOffset));
                OPB->draw(points[0], DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset));
                OPB->end();

                if (!isMultiSelection())
                {
                    auto spriteTransform = pVideo->GetTransform();
                    OSB->begin(transform);
                    OSB->drawCross(spriteTransform.Translation(), 10.f, HANDLE_COLOR);
                    OSB->end();
                }
            }
            else
            {
                OPB->begin(onut::ePrimitiveType::LINE_STRIP, pDottedLineTexture);
                OPB->draw(gizmo.transformHandles[0].screenPos, DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset));
                OPB->draw(gizmo.transformHandles[2].screenPos, DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset * dottedLineScale * pNode->GetScale().y));
                OPB->draw(gizmo.transformHandles[4].screenPos, DOTTED_LINE_COLOR, Vector2(dottedLineOffset * dottedLineScale * pNode->GetScale().x, dottedLineOffset * dottedLineScale * pNode->GetScale().y));
                OPB->draw(gizmo.transformHandles[6].screenPos, DOTTED_LINE_COLOR, Vector2(dottedLineOffset * dottedLineScale * pNode->GetScale().x, dottedLineOffset));
                OPB->draw(gizmo.transformHandles[0].screenPos, DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset));
                OPB->end();
                OSB->begin(transform);

                auto spriteTransform = pNode->GetTransform();
                OSB->drawCross(spriteTransform.Translation(), 10.f, HANDLE_COLOR);
                OSB->end();
            }
        }
        if (isMultiSelection())
        {
            OPB->begin(onut::ePrimitiveType::LINE_STRIP, pDottedLineTexture);
            OPB->draw(gizmo.aabb[0], AABB_DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset));
            OPB->draw(Vector2(gizmo.aabb[0].x, gizmo.aabb[1].y), AABB_DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset + (gizmo.aabb[1].y - gizmo.aabb[0].y) * dottedLineScale));
            OPB->draw(gizmo.aabb[1], AABB_DOTTED_LINE_COLOR, Vector2(dottedLineOffset + (gizmo.aabb[1].x - gizmo.aabb[0].x) * dottedLineScale, dottedLineOffset + (gizmo.aabb[1].y - gizmo.aabb[0].y) * dottedLineScale));
            OPB->draw(Vector2(gizmo.aabb[1].x, gizmo.aabb[0].y), AABB_DOTTED_LINE_COLOR, Vector2(dottedLineOffset + (gizmo.aabb[1].x - gizmo.aabb[0].x) * dottedLineScale, dottedLineOffset));
            OPB->draw(gizmo.aabb[0], AABB_DOTTED_LINE_COLOR, Vector2(dottedLineOffset, dottedLineOffset));
            OPB->end();
        }

        // Handles
        OSB->begin();
        for (auto& handle : gizmo.transformHandles)
        {
            OSB->drawSprite(nullptr, handle.screenPos, HANDLE_COLOR, 0, 6);
        }
    });

    // Mouse down for select
    ui_mainView->onMouseLeave = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) 
    {
        if (state == State::Idle)
        {
            OWindow->setCursor(curARROW);
        }
    };

    ui_treeView->onSelectionChanged = [](onut::UITreeView* in_pTreeView, const onut::UITreeViewSelectEvent& event)
    {
        auto selectionBefore = selection;
        auto selectionAfter = selection;
        selectionAfter.clear();
        for (auto pItem : *event.pSelectedItems)
        {
            auto pContainer = std::static_pointer_cast<NodeContainer>(pItem->pSharedUserData);
            if (pContainer)
            {
                if (pContainer->pNode == pEditingView->GetRootNode()) continue;
                selectionAfter.push_back(pContainer);
            }
        }
        if (selectionBefore != selectionAfter)
        {
            actionManager.doAction(new onut::Action("Select",
                [=]
            {
                selection = selectionAfter;
                updateProperties();
                markModified();
            }, [=]
            {
                selection = selectionBefore;
                updateProperties();
                markModified();
            }, [=]
            {
            }, [=]
            {
            }));
        }
    };

    ui_mainView->onMouseDown = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        mousePosOnDown = event.mousePos;
        auto bMouseInSelection = isMouseInSelection(event.mousePos);

        // Check handles
        if (!selection.empty() && !OPressed(OINPUT_LCONTROL))
        {
            HandleIndex index = 0;
            HandleIndex closestIndex = 0;
            float closestDis = Vector2::DistanceSquared(onut::UI2Onut(mousePosOnDown), gizmo.transformHandles.front().screenPos);
            for (auto& handle : gizmo.transformHandles)
            {
                float dis = Vector2::DistanceSquared(onut::UI2Onut(mousePosOnDown), handle.screenPos);
                if (dis < closestDis)
                {
                    closestDis = dis;
                    closestIndex = index;
                }
                ++index;
            }
            if (closestDis < 8.f * 8.f && !isMultiSelection())
            {
                state = State::IsAboutToMoveHandle;
                handleIndexOnDown = closestIndex;
            }
            else if (bMouseInSelection)
            {
                state = State::IsAboutToMove;
            }
            else if (closestDis < 32.f * 32.f)
            {
                state = State::IsAboutToRotate;
                handleIndexOnDown = closestIndex;
            }

            if (state == State::IsAboutToMove ||
                state == State::IsAboutToRotate)
            {
                // We will not include the children of a selected node in a move operation
                std::unordered_set<std::shared_ptr<NodeContainer>> parentSet;
                cleanedUpSelection.clear();
                for (auto pContainer : selection)
                {
                    if (parentSet.find(pContainer) != parentSet.end()) continue; // Already in there as a child
                    collectParentsOnly(pContainer, parentSet);
                    cleanedUpSelection.push_back(pContainer);
                }
            }
        }

        if (state == State::Idle)
        {
            seed::Node* pMouseHover = nullptr;

            // Find the topmost mouse hover sprite
            pEditingView->VisitNodesBackward([&](seed::Node* pNode) -> bool
            {
                if (!pNode->GetReallyVisible()) return false;
                if (mouseInSprite(event.mousePos, pNode))
                {
                    pMouseHover = pNode;
                    return true;
                }
                return false;
            });

            // We ignore root node
            if (pMouseHover == pEditingView->GetRootNode())
            {
                pMouseHover = nullptr;
            }

            // Add to selection
            if (pMouseHover)
            {
                if (OPressed(OINPUT_LCONTROL))
                {
                    auto selectionBefore = selection;
                    auto selectionAfter = selection;
                    bool found = false;
                    for (auto it = selectionAfter.begin(); it != selectionAfter.end(); ++it)
                    {
                        if ((*it)->pNode == pMouseHover)
                        {
                            found = true;
                            selectionAfter.erase(it);
                            break;
                        }
                    }
                    auto pContainer = nodesToContainers[pMouseHover];
                    if (!found)
                    {
                        selectionAfter.push_back(pContainer);
                    }
                    actionManager.doAction(new onut::Action("Select",
                        [=]
                    {
                        selection = selectionAfter;
                        updateProperties();
                        markModified();
                    }, [=]
                    {
                        selection = selectionBefore;
                        updateProperties();
                        markModified();
                    }));
                }
                else
                {
                    state = State::IsAboutToMove;
                    bool found = false;
                    auto pContainer = nodesToContainers[pMouseHover];
                    auto selectionBefore = selection;
                    auto selectionAfter = selection;
                    selectionAfter.clear();
                    selectionAfter.push_back(pContainer);
                    cleanedUpSelection = selectionAfter;
                    actionManager.doAction(new onut::Action("Select",
                        [=]
                    {
                        selection = selectionAfter;
                        updateProperties();
                        markModified();
                    }, [=]
                    {
                        selection = selectionBefore;
                        updateProperties();
                        markModified();
                    }));
                }
            }
            else
            {
                // Unselect all if we are not within it's aabb
                auto selectionBefore = selection;
                auto selectionAfter = selection;
                selectionAfter.clear();
                actionManager.doAction(new onut::Action("Unselect",
                    [=]
                {
                    selection = selectionAfter;
                    updateProperties();
                    markModified();
                }, [=]
                {
                    selection = selectionBefore;
                    updateProperties();
                    markModified();
                }));
            }
        }
    };

    ui_mainView->onMouseMove = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        if (state == State::Panning)
        {
            Vector2 mouseDiff = Vector2((float)(event.mousePos.x - mousePosOnDown.x), (float)(event.mousePos.y - mousePosOnDown.y));
            mouseDiff /= zoom;
            cameraPos = cameraPosOnDown - mouseDiff;
            updateProperties();
            return;
        }

        auto mouseDiff = onut::UI2Onut(event.mousePos) - onut::UI2Onut(mousePosOnDown);
        mouseDiff /= zoom;
        checkAboutToAction(State::IsAboutToMove, State::Moving, mouseDiff);
        checkAboutToAction(State::IsAboutToMoveHandle, State::MovingHandle, mouseDiff);
        checkAboutToAction(State::IsAboutToRotate, State::Rotate, mouseDiff);
        if (state == State::Moving)
        {
            if (OPressed(OINPUT_LSHIFT))
            {
                if (std::abs(mouseDiff.x) > std::abs(mouseDiff.y))
                {
                    mouseDiff.y = 0;
                }
                else
                {
                    mouseDiff.x = 0;
                }
            }
            for (auto pContainer : cleanedUpSelection)
            {
                auto transform = pContainer->stateOnDown->node.parentTransform;
                auto worldPos = Vector2::Transform(pContainer->stateOnDown->node.position, transform);
                auto invTransform = transform.Invert();
                worldPos += mouseDiff;
                pContainer->pNode->SetPosition(Vector2::Transform(worldPos, invTransform));
            }
            updateProperties();
        }
        else if (state == State::MovingHandle)
        {
            auto& handle = gizmo.transformHandles[handleIndexOnDown];
            auto pContainer = selection.front();
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);

            Vector2 size(32, 32);
            if (pSprite || pVideo) size = Vector2(pContainer->pNode->GetWidth(), pContainer->pNode->GetHeight());

            auto invTransform = pContainer->stateOnDown->node.transform.Invert();
            invTransform._41 = 0;
            invTransform._42 = 0;

            auto localMouseDiff = Vector2::Transform(mouseDiff, invTransform);
            auto localScaleDiff = handle.transformDirection * localMouseDiff;
            Vector2 newScale;
            if (OPressed(OINPUT_LSHIFT))
            {
                localScaleDiff.x = localScaleDiff.y = std::max<>(localScaleDiff.x, localScaleDiff.y);
                newScale = pContainer->stateOnDown->node.scale + localScaleDiff / size * 2.f * pContainer->stateOnDown->node.scale;
                auto ratioOnDown = pContainer->stateOnDown->node.scale.x / pContainer->stateOnDown->node.scale.y;
                newScale.y = newScale.x / ratioOnDown;
            }
            else
            {
                newScale = pContainer->stateOnDown->node.scale + localScaleDiff / size * 2.f * pContainer->stateOnDown->node.scale;
            }

            pContainer->pNode->SetScale(newScale);
            updateProperties();
        }
        else if (state == State::Rotate)
        {
            auto diff1 = onut::UI2Onut(mousePosOnDown) - selectionCenter;
            auto diff2 = onut::UI2Onut(event.mousePos) - selectionCenter;
            auto angle1 = DirectX::XMConvertToDegrees(std::atan2f(diff1.y, diff1.x));
            auto angle2 = DirectX::XMConvertToDegrees(std::atan2f(diff2.y, diff2.x));
            auto angleDiff = angle2 - angle1;
            if (OPressed(OINPUT_LSHIFT))
            {
                angleDiff = std::round(angleDiff / 5.f) * 5.f;
            }
            if (isMultiSelection())
            {
                auto viewTransform = getViewTransform();
                auto invViewTransform = viewTransform.Invert();
                for (auto pContainer : cleanedUpSelection)
                {
                    auto screenPosition = Vector2::Transform(pContainer->stateOnDown->node.transform.Translation(), viewTransform);
                    auto centerVect = screenPosition - selectionCenter;
                    centerVect = Vector2::Transform(centerVect, Matrix::CreateRotationZ(DirectX::XMConvertToRadians(angleDiff)));
                    screenPosition = centerVect + selectionCenter;
                    auto viewPosition = Vector2::Transform(screenPosition, invViewTransform);
                    auto invParentTransform = pContainer->stateOnDown->node.parentTransform.Invert();
                    auto localPosition = Vector2::Transform(viewPosition, invParentTransform);
                    pContainer->pNode->SetPosition(localPosition);
                    pContainer->pNode->SetAngle(pContainer->stateOnDown->node.angle + angleDiff);
                }
                switch (gizmo.transformHandles[handleIndexOnDown].handle)
                {
                    case Handle::TOP_LEFT:
                    case Handle::BOTTOM_RIGHT:
                        OWindow->setCursor(directionCursor(curSIZENESW, angleDiff));
                        break;
                    case Handle::LEFT:
                    case Handle::RIGHT:
                        OWindow->setCursor(directionCursor(curSIZENS, angleDiff));
                        break;
                    case Handle::BOTTOM_LEFT:
                    case Handle::TOP_RIGHT:
                        OWindow->setCursor(directionCursor(curSIZENWSE, angleDiff));
                        break;
                    case Handle::BOTTOM:
                    case Handle::TOP:
                        OWindow->setCursor(directionCursor(curSIZEWE, angleDiff));
                        break;
                }
            }
            else
            {
                auto pContainer = selection.front();
                pContainer->pNode->SetAngle(pContainer->stateOnDown->node.angle + angleDiff);
                switch (gizmo.transformHandles[handleIndexOnDown].handle)
                {
                    case Handle::TOP_LEFT:
                    case Handle::BOTTOM_RIGHT:
                        OWindow->setCursor(directionCursor(curSIZENESW, pContainer->pNode->GetTransform()));
                        break;
                    case Handle::LEFT:
                    case Handle::RIGHT:
                        OWindow->setCursor(directionCursor(curSIZENS, pContainer->pNode->GetTransform()));
                        break;
                    case Handle::BOTTOM_LEFT:
                    case Handle::TOP_RIGHT:
                        OWindow->setCursor(directionCursor(curSIZENWSE, pContainer->pNode->GetTransform()));
                        break;
                    case Handle::BOTTOM:
                    case Handle::TOP:
                        OWindow->setCursor(directionCursor(curSIZEWE, pContainer->pNode->GetTransform()));
                        break;
                }
            }
            updateProperties();
        }
        else if (state == State::Idle)
        {
            // Change cursor on depending what we hover
            auto cursor = curARROW;

            // Check handles
            if (!selection.empty() && !OPressed(OINPUT_LCONTROL))
            {
                HandleIndex index = 0;
                HandleIndex closestIndex = 0;
                float closestDis = Vector2::DistanceSquared(onut::UI2Onut(event.mousePos), gizmo.transformHandles.front().screenPos);
                for (auto& handle : gizmo.transformHandles)
                {
                    float dis = Vector2::DistanceSquared(onut::UI2Onut(event.mousePos), handle.screenPos);
                    if (dis < closestDis)
                    {
                        closestDis = dis;
                        closestIndex = index;
                    }
                    ++index;
                }
                bool isHandle = false;
                if (closestDis < 8.f * 8.f && !isMultiSelection())
                {
                    isHandle = true;
                    switch (gizmo.transformHandles[closestIndex].handle)
                    {
                        case Handle::TOP_LEFT:
                        case Handle::BOTTOM_RIGHT:
                            cursor = directionCursor(curSIZENWSE, selection.front()->pNode->GetTransform());
                            break;
                        case Handle::LEFT:
                        case Handle::RIGHT:
                            cursor = directionCursor(curSIZEWE, selection.front()->pNode->GetTransform());
                            break;
                        case Handle::BOTTOM_LEFT:
                        case Handle::TOP_RIGHT:
                            cursor = directionCursor(curSIZENESW, selection.front()->pNode->GetTransform());
                            break;
                        case Handle::BOTTOM:
                        case Handle::TOP:
                            cursor = directionCursor(curSIZENS, selection.front()->pNode->GetTransform());
                            break;
                    }
                }
                else if (closestDis < 32.f * 32.f)
                {
                    if (!isMultiSelection())
                    {
                        switch (gizmo.transformHandles[closestIndex].handle)
                        {
                            case Handle::TOP_LEFT:
                            case Handle::BOTTOM_RIGHT:
                                cursor = directionCursor(curSIZENESW, selection.front()->pNode->GetTransform());
                                break;
                            case Handle::LEFT:
                            case Handle::RIGHT:
                                cursor = directionCursor(curSIZENS, selection.front()->pNode->GetTransform());
                                break;
                            case Handle::BOTTOM_LEFT:
                            case Handle::TOP_RIGHT:
                                cursor = directionCursor(curSIZENWSE, selection.front()->pNode->GetTransform());
                                break;
                            case Handle::BOTTOM:
                            case Handle::TOP:
                                cursor = directionCursor(curSIZEWE, selection.front()->pNode->GetTransform());
                                break;
                        }
                    }
                    else
                    {
                        switch (gizmo.transformHandles[closestIndex].handle)
                        {
                            case Handle::TOP_LEFT:
                            case Handle::BOTTOM_RIGHT:
                                cursor = curSIZENESW;
                                break;
                            case Handle::LEFT:
                            case Handle::RIGHT:
                                cursor = curSIZENS;
                                break;
                            case Handle::BOTTOM_LEFT:
                            case Handle::TOP_RIGHT:
                                cursor = curSIZENWSE;
                                break;
                            case Handle::BOTTOM:
                            case Handle::TOP:
                                cursor = curSIZEWE;
                                break;
                        }
                    }
                }
                if (!isHandle)
                {
                    if (isMultiSelection())
                    {
                        auto aabb = getSelectionAABB();
                        if (event.mousePos.x >= aabb[0].x &&
                            event.mousePos.y >= aabb[0].y &&
                            event.mousePos.x <= aabb[1].x &&
                            event.mousePos.y <= aabb[1].y)
                        {
                            cursor = curSIZEALL;
                        }
                    }
                    else
                    {
                        if (mouseInSprite(event.mousePos, selection.front()))
                        {
                            cursor = curSIZEALL;
                        }
                    }
                }
            }

            OWindow->setCursor(cursor);
        }
    };

    ui_mainView->onMouseUp = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        finalizeAction("Move", State::Moving);
        finalizeAction("Scale", State::MovingHandle);
        finalizeAction("Rotate", State::Rotate);
        state = State::Idle;
        OWindow->setCursor(curARROW);
    };

    OFindUI("btnStartEmitter")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        startSelectedEmitters();
    };

    OFindUI("btnStopEmitter")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        for (auto pContainer : selection)
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            if (pEmitter)
            {
                pEmitter->Stop();
            }
        }
    };

    OFindUI("btnPlaySoundEmitter")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        startSelectedSoundEmitters();
    };

    OFindUI("btnStopSoundEmitter")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        for (auto pContainer : selection)
        {
            auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
            if (pSoundEmitter)
            {
                pSoundEmitter->Stop();
            }
        }
    };

    OFindUI("btnPlayMusicEmitter")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        startSelectedMusicEmitters();
    };

    OFindUI("btnStopMusicEmitter")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        for (auto pContainer : selection)
        {
            auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
            if (pMusicEmitter)
            {
                pMusicEmitter->Stop();
            }
        }
    };

    OFindUI("btnPlayVideo")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        startSelectedVideos();
    };

    OFindUI("btnStopVideo")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        for (auto pContainer : selection)
        {
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
            if (pVideo)
            {
                pVideo->Stop();
            }
        }
    };

    updateProperties();

    auto lastFile = OSettings->getUserSetting("lastFile");
    if (!lastFile.empty())
    {
        if (onut::fileExists(lastFile))
        {
            onOpen(lastFile);
        }
    }

    OWindow->onDrop = [](const std::string& fullPath)
    {
        auto filename = onut::getFilename(fullPath);
        auto extension = onut::getExtension(filename);
        if (extension == "PNG")
        {
            createSprite(filename);
            onMenu(MENU_EDIT_FOCUS_SELECTION);
        }
        else if (extension == "PEX" ||
                 extension == "PFX")
        {
            createEmitter(filename);
            startSelectedEmitters();
            onMenu(MENU_EDIT_FOCUS_SELECTION);
        }
        else if (extension == "FNT")
        {
            createSpriteString(filename);
            onMenu(MENU_EDIT_FOCUS_SELECTION);
        }
        else if (extension == "WAV" ||
                 extension == "CUE")
        {
            createSoundEmitter(filename);
            onMenu(MENU_EDIT_FOCUS_SELECTION);
        }
        else if (extension == "MP3")
        {
            createMusicEmitter(filename);
            onMenu(MENU_EDIT_FOCUS_SELECTION);
        }
        else if (extension == "MP4")
        {
            createVideo(filename);
            onMenu(MENU_EDIT_FOCUS_SELECTION);
        }
        else if (extension == "TMX")
        {
            createTiledMapNode(filename);
            onMenu(MENU_EDIT_FOCUS_SELECTION);
        }
    };

    OWindow->onResize = [](const POINT& newSize)
    {
        updateTransformHandles();
    };

    OFindUI("btnToolNew")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) {onMenu(MENU_FILE_NEW); };
    OFindUI("btnToolOpen")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) {onMenu(MENU_FILE_OPEN); };
    OFindUI("btnToolSave")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) {onMenu(MENU_FILE_SAVE); };

    OFindUI("btnToolCut")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) {onMenu(MENU_EDIT_CUT); };
    OFindUI("btnToolCopy")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) {onMenu(MENU_EDIT_COPY); };
    OFindUI("btnToolPaste")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) {onMenu(MENU_EDIT_PASTE); };
    OFindUI("btnToolDelete")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) {onMenu(MENU_EDIT_DELETE); };

    OFindUI("btnToolUndo")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) {onMenu(MENU_EDIT_UNDO); };
    OFindUI("btnToolRedo")->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event) {onMenu(MENU_EDIT_REDO); };
}

void update()
{
    if (pEditingView)
    {
        pEditingView->Update();
    }

    if (state == State::Idle)
    {
        // Zoom
        if (OUIContext->getHoverControl() == ui_mainView)
        {
            if (OInput->getStateValue(OINPUT_MOUSEZ) < 0)
            {
                --zoomIndex;
                if (zoomIndex < 0) zoomIndex = 0;
                zoom = zoomLevels[zoomIndex];
                updateProperties();
            }
            else if (OInput->getStateValue(OINPUT_MOUSEZ) > 0)
            {
                ++zoomIndex;
                if (zoomIndex > (ZoomIndex)zoomLevels.size() - 1) zoomIndex = (ZoomIndex)zoomLevels.size() - 1;
                zoom = zoomLevels[zoomIndex];
                updateProperties();
            }
        }
    }
    else if (state == State::Moving ||
             state == State::Rotate ||
             state == State::MovingHandle)
    {
        if (OJustPressed(OINPUT_MOUSEB2))
        {
            // cancel!!!
            state = State::Idle;
            for (auto pContainer : selection)
            {
                pContainer->stateOnDown->apply();
            }
            updateProperties();
        }
    }
}

bool hasVideoPlaying()
{
    if (!pEditingView) return false;
    bool ret = false;
    pEditingView->VisitNodes([&](seed::Node *pNode) -> bool
    {
        auto pVideo = dynamic_cast<seed::Video*>(pNode);
        if (pVideo)
        {
            if (pVideo->IsPlaying())
            {
                ret = true;
                return true;
            }
        }
        return false;
    });
    return ret;
}

void render()
{
    static const std::string spinner[] = {"/", "-", "\\", "|"};
    static int spinnerIndex = 0;
    spinnerIndex = (spinnerIndex + 1) % 4;
    ((onut::UILabel*)OFindUI("renderSpinner"))->textComponent.text = spinner[spinnerIndex];

    if (!OSettings->getIsEditorMode())
    {
        if (!OParticles->hasAliveParticles() &&
            !hasVideoPlaying())
        {
            OSettings->setIsEditorMode(true);
        }
    }
}
