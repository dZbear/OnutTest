// Required for WinMain
#include <Windows.h>

// Oak Nut include
#include "onut.h"
#include "ActionManager.h"
#include "menu.h"
#include "NodeContainer.h"
#include "tinyxml2.h"

#include <unordered_set>
#include <unordered_map>

void createUIStyles(onut::UIContext* pContext);
void init();
void update();
void render();

enum class State
{
    Idle,
    Panning,
    Zooming,
    Moving,
    MovingHandle,
    Rotate,
    IsAboutToRotate,
    IsAboutToMove,
    IsAboutToMoveHandle,
};

enum class Handle
{
    TOP_LEFT,
    LEFT,
    BOTTOM_LEFT,
    BOTTOM,
    BOTTOM_RIGHT,
    RIGHT,
    TOP_RIGHT,
    TOP
};

struct TransformHandle
{
    Vector2 screenPos;
    Handle handle = Handle::TOP_LEFT;
    Vector2 transformDirection;
};

using TransformHandles = std::vector<TransformHandle>;
using AABB = std::vector<Vector2>;
using HandleIndex = AABB::size_type;

struct Gizmo
{
    TransformHandles transformHandles;
    AABB aabb;
};

// Utilities
onut::ActionManager actionManager;
std::unordered_map<seed::Node*, std::shared_ptr<NodeContainer>> nodesToContainers;

// Camera
using Zoom = float;
using ZoomIndex = int;
static const std::vector<Zoom> zoomLevels = {.20f, .50f, .70f, 1.f, 1.5f, 2.f, 4.f};
ZoomIndex zoomIndex = 3;
Zoom zoom = zoomLevels[zoomIndex];
Vector2 cameraPos = Vector2::Zero;

// Selection
using Selection = std::vector<std::shared_ptr<NodeContainer>>;
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
using Clipboard = std::vector<NodeState>;
Clipboard clipboard;

// Controls
onut::UIControl* ui_mainView = nullptr;
onut::UITreeView* ui_treeView = nullptr;
onut::UITreeViewItem* pTreeViewRoot = nullptr;

onut::UIControl* ui_propertiesView = nullptr;
onut::UITextBox* ui_txtViewWidth = nullptr;
onut::UITextBox* ui_txtViewHeight = nullptr;

onut::UIButton* ui_btnCreateNode = nullptr;
onut::UIControl* ui_propertiesNode = nullptr;
onut::UITextBox* ui_txtSpriteName = nullptr;
onut::UITextBox* ui_txtSpriteType = nullptr;
onut::UICheckBox* ui_chkNodeVisible = nullptr;
onut::UITextBox* ui_txtSpriteX = nullptr;
onut::UITextBox* ui_txtSpriteY = nullptr;
onut::UITextBox* ui_txtSpriteScaleX = nullptr;
onut::UITextBox* ui_txtSpriteScaleY = nullptr;
onut::UITextBox* ui_txtSpriteAngle = nullptr;
onut::UIPanel* ui_colSpriteColor = nullptr;
onut::UITextBox* ui_txtSpriteAlpha = nullptr;

onut::UIButton* ui_btnCreateSprite = nullptr;
onut::UIControl* ui_propertiesSprite = nullptr;
onut::UITextBox* ui_txtSpriteTexture = nullptr;
onut::UIButton* ui_btnSpriteTextureBrowse = nullptr;
onut::UITextBox* ui_txtSpriteAlignX = nullptr;
onut::UITextBox* ui_txtSpriteAlignY = nullptr;
onut::UICheckBox* ui_chkSpriteFlippedH = nullptr;
onut::UICheckBox* ui_chkSpriteFlippedV = nullptr;
onut::UICheckBox* ui_chkSpriteBlends[4] = {nullptr};
onut::UICheckBox* ui_chkSpriteFilters[2] = {nullptr};

onut::UIButton* ui_btnCreateSpriteString = nullptr;
onut::UIControl* ui_propertiesSpriteString = nullptr;
onut::UITextBox* ui_txtSpriteStringFont = nullptr;
onut::UIButton* ui_btnSpriteStringFontBrowse = nullptr;
onut::UITextBox* ui_txtSpriteStringCaption = nullptr;

onut::UIButton* ui_btnCreateEmitter = nullptr;
onut::UIControl* ui_propertiesEmitter = nullptr;
onut::UITextBox* ui_txtEmitterFx = nullptr;
onut::UIButton* ui_txtEmitterFxBrowse = nullptr;
onut::UICheckBox* ui_chkEmitterBlends[4] = {nullptr};
onut::UICheckBox* ui_chkEmitterFilters[2] = {nullptr};
onut::UICheckBox* ui_chkEmitterEmitWorld = nullptr;

onut::UIControl* ui_propertiesSoundEmitter = nullptr;
onut::UIButton* ui_btnCreateSoundEmitter = nullptr;
onut::UITextBox* ui_txtSoundEmitter = nullptr;
onut::UIButton* ui_btnSoundEmitterBrowse = nullptr;
onut::UICheckBox* ui_chkSoundEmitterLoop = nullptr;
onut::UICheckBox* ui_chkSoundEmitterPositionBased = nullptr;
onut::UITextBox* ui_txtSoundEmitterVolume = nullptr;
onut::UITextBox* ui_txtSoundEmitterBalance = nullptr;
onut::UITextBox* ui_txtSoundEmitterPitch = nullptr;

onut::UIControl* ui_propertiesMusicEmitter = nullptr;
onut::UIButton* ui_btnCreateMusicEmitter = nullptr;
onut::UITextBox* ui_txtMusicEmitter = nullptr;
onut::UIButton* ui_btnMusicEmitterBrowse = nullptr;
onut::UICheckBox* ui_chkMusicEmitterLoop = nullptr;
onut::UITextBox* ui_txtMusicEmitterVolume = nullptr;

onut::UIControl* ui_propertiesVideo = nullptr;
onut::UIButton* ui_btnCreateVideo = nullptr;
onut::UITextBox* ui_txtVideo = nullptr;
onut::UIButton* ui_btnVideoBrowse = nullptr;
onut::UICheckBox* ui_chkVideoLoop = nullptr;
onut::UITextBox* ui_txtVideoVolume = nullptr;
onut::UITextBox* ui_txtVideoPlayRate = nullptr;
onut::UITextBox* ui_txtVideoWidth = nullptr;
onut::UITextBox* ui_txtVideoHeight = nullptr;

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

void updateProperties()
{
    ui_treeView->unselectAll();

    ui_propertiesView->isVisible = false;
    ui_propertiesNode->isVisible = false;
    ui_propertiesSprite->isVisible = false;
    ui_propertiesSpriteString->isVisible = false;
    ui_propertiesEmitter->isVisible = false;
    ui_propertiesSoundEmitter->isVisible = false;
    ui_propertiesMusicEmitter->isVisible = false;
    ui_propertiesVideo->isVisible = false;

    ui_btnCreateSpriteString->isEnabled = false;
    ui_btnCreateSprite->isEnabled = false;
    ui_btnCreateNode->isEnabled = false;
    ui_btnCreateEmitter->isEnabled = false;
    ui_btnCreateSoundEmitter->isEnabled = false;
    ui_btnCreateMusicEmitter->isEnabled = false;
    ui_btnCreateVideo->isEnabled = false;
    ui_mainView->isEnabled = false;

    if (!pEditingView) return;

    ui_btnCreateSpriteString->isEnabled = true;
    ui_btnCreateSprite->isEnabled = true;
    ui_btnCreateNode->isEnabled = true;
    ui_btnCreateEmitter->isEnabled = true;
    ui_btnCreateSoundEmitter->isEnabled = true;
    ui_btnCreateMusicEmitter->isEnabled = true;
    ui_btnCreateVideo->isEnabled = true;
    ui_mainView->isEnabled = true;

    if (selection.empty())
    {
        ui_propertiesView->isVisible = true;
        ui_txtViewWidth->setInt((int)viewSize.x);
        ui_txtViewHeight->setInt((int)viewSize.y);
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

        if (pNode)
        {
            ui_propertiesNode->isVisible = true;
            ui_txtSpriteName->textComponent.text = pContainer->pNode->GetName();
            ui_txtSpriteType->textComponent.text = "";
            ui_txtSpriteX->setFloat(pContainer->pNode->GetPosition().x);
            ui_txtSpriteY->setFloat(pContainer->pNode->GetPosition().y);
            ui_txtSpriteScaleX->setFloat(pContainer->pNode->GetScale().x);
            ui_txtSpriteScaleY->setFloat(pContainer->pNode->GetScale().y);
            ui_txtSpriteAngle->setFloat(pContainer->pNode->GetAngle());
            ui_colSpriteColor->color = onut::sUIColor(pContainer->pNode->GetColor().x, pContainer->pNode->GetColor().y, pContainer->pNode->GetColor().z, pContainer->pNode->GetColor().w);
            ui_txtSpriteAlpha->setFloat(pContainer->pNode->GetColor().w * 100.f);
            ui_chkNodeVisible->setIsChecked(pNode->GetVisible());
        }

        if (pSprite)
        {
            ui_propertiesSprite->isVisible = true;
            auto pTexture = pSprite->GetTexture();
            if (pTexture)
            {
                ui_txtSpriteTexture->textComponent.text = pTexture->getName();
            }
            else
            {
                ui_txtSpriteTexture->textComponent.text = "";
            }
            ui_txtSpriteAlignX->setFloat(pSprite->GetAlign().x);
            ui_txtSpriteAlignY->setFloat(pSprite->GetAlign().y);
            ui_chkSpriteFlippedH->setIsChecked(pSprite->GetFlippedH());
            ui_chkSpriteFlippedV->setIsChecked(pSprite->GetFlippedV());
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
                ui_propertiesSpriteString->isVisible = true;
                auto pFont = pSpriteString->GetFont();
                if (pFont)
                {
                    ui_txtSpriteStringFont->textComponent.text = pFont->getName();
                }
                else
                {
                    ui_txtSpriteStringFont->textComponent.text = "";
                }
                ui_txtSpriteStringCaption->textComponent.text = pSpriteString->GetCaption();
            }
        }
        else if (pEmitter)
        {
            ui_propertiesEmitter->isVisible = true;
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
            ui_chkEmitterEmitWorld->setIsChecked(pEmitter->GetEmitWorld());
        }
        else if (pSoundEmitter)
        {
            ui_propertiesSoundEmitter->isVisible = true;
            ui_txtSoundEmitter->textComponent.text = pSoundEmitter->GetSource();
            ui_txtSoundEmitterVolume->setFloat(pSoundEmitter->GetVolume() * 100.f);
            ui_txtSoundEmitterBalance->setFloat(pSoundEmitter->GetBalance() * 100.f);
            ui_txtSoundEmitterPitch->setFloat(pSoundEmitter->GetPitch() * 100.f);
            ui_chkSoundEmitterPositionBased->setIsChecked(pSoundEmitter->GetPositionBasedBalance() || pSoundEmitter->GetPositionBasedBalance());
            ui_chkSoundEmitterLoop->setIsChecked(pSoundEmitter->GetLoops());
        }
        else if (pMusicEmitter)
        {
            ui_propertiesMusicEmitter->isVisible = true;
            ui_txtMusicEmitter->textComponent.text = pMusicEmitter->GetSource();
            ui_txtMusicEmitterVolume->setFloat(pMusicEmitter->GetVolume() * 100.f);
            ui_chkMusicEmitterLoop->setIsChecked(pMusicEmitter->GetLoops());
        }
        else if (pVideo)
        {
            ui_propertiesVideo->isVisible = true;
            ui_txtVideo->textComponent.text = pVideo->GetSource();
            ui_txtVideoVolume->setFloat(pVideo->GetVolume() * 100.f);
            ui_txtVideoPlayRate->setFloat((float)pVideo->GetPlayRate() * 100.f);
            ui_chkVideoLoop->setIsChecked(pVideo->GetLoops());
            ui_txtVideoWidth->setFloat(pVideo->GetDimensions().x);
            ui_txtVideoHeight->setFloat(pVideo->GetDimensions().y);
        }

        ui_treeView->expandTo(pContainer->pTreeViewItem);
        ui_treeView->addSelectedItem(pContainer->pTreeViewItem);
    }

    updateTransformHandles();
}

// Main
int CALLBACK WinMain(HINSTANCE appInstance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdCount)
{
    // Set default settings
    OSettings->setGameName("Seed Editor");
    OSettings->setIsResizableWindow(true);
    OSettings->setResolution({1280, 720});
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
        NodeState* spriteStateBefore = new NodeState(pContainer);
        logic(pContainer);
        NodeState* spriteStateAfter = new NodeState(pContainer);
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
        },
            [=]
        {
        }, [=]
        {
            delete spriteStateBefore;
            delete spriteStateAfter;
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
                pContainer->stateOnDown = NodeState(pContainer);
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
            auto stateBefore = new NodeState(pContainer->stateOnDown);
            auto stateAfter = new NodeState(pContainer);
            pGroup->addAction(new onut::Action("",
                [=]{
                stateAfter->apply();
                updateProperties();
                markModified();
            }, [=] {
                stateBefore->apply();
                updateProperties();
                markModified();
            }, [=] {
            }, [=] {
                delete stateBefore;
                delete stateAfter;
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
            auto worldPos = Vector2::Transform(spriteState.position, spriteState.parentTransform);
            auto invTransform = spriteState.parentTransform.Invert();
            worldPos.x -= step;
            pContainer->pNode->SetPosition(Vector2::Transform(worldPos, invTransform));
        });
    }
    else if (key == VK_RIGHT)
    {
        changeSpriteProperty("Nudge", [=](std::shared_ptr<NodeContainer> pContainer)
        {
            NodeState spriteState(pContainer);
            auto worldPos = Vector2::Transform(spriteState.position, spriteState.parentTransform);
            auto invTransform = spriteState.parentTransform.Invert();
            worldPos.x += step;
            pContainer->pNode->SetPosition(Vector2::Transform(worldPos, invTransform));
        });
    }
    if (key == VK_UP)
    {
        changeSpriteProperty("Nudge", [=](std::shared_ptr<NodeContainer> pContainer)
        {
            NodeState spriteState(pContainer);
            auto worldPos = Vector2::Transform(spriteState.position, spriteState.parentTransform);
            auto invTransform = spriteState.parentTransform.Invert();
            worldPos.y -= step;
            pContainer->pNode->SetPosition(Vector2::Transform(worldPos, invTransform));
        });
    }
    else if (key == VK_DOWN)
    {
        changeSpriteProperty("Nudge", [=](std::shared_ptr<NodeContainer> pContainer)
        {
            NodeState spriteState(pContainer);
            auto worldPos = Vector2::Transform(spriteState.position, spriteState.parentTransform);
            auto invTransform = spriteState.parentTransform.Invert();
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

        auto pStateBefore = new NodeState(pContainer, true);

        pGroup->addAction(new onut::Action("",
            [=]{ // OnRedo
            pEditingView->DeleteNode(pContainer->pNode);
            pParentContainer->pTreeViewItem->removeItem(pContainer->pTreeViewItem);
            pStateBefore->visit([](NodeState* pNodeState)
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
        },
            [=]{ // Init
        },
            [=]{ // Destroy
            delete pStateBefore;
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
        nodeState.visit([](NodeState* pNodeState)
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
            state.visit([](NodeState *pNodeState)
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

void init()
{
    curARROW = LoadCursor(nullptr, IDC_ARROW);
    curSIZENWSE = LoadCursor(nullptr, IDC_SIZENWSE);
    curSIZENESW = LoadCursor(nullptr, IDC_SIZENESW);
    curSIZEWE = LoadCursor(nullptr, IDC_SIZEWE);
    curSIZENS = LoadCursor(nullptr, IDC_SIZENS);
    curSIZEALL = LoadCursor(nullptr, IDC_SIZEALL);

    createUIStyles(OUIContext);
    OUI->add(OLoadUI("editor.json"));

    ui_mainView = OFindUI("mainView");
    ui_treeView = dynamic_cast<onut::UITreeView*>(OFindUI("treeView"));
    ui_treeView->allowReorder = true;

    ui_propertiesView = OFindUI("propertiesView");
    ui_txtViewWidth = dynamic_cast<onut::UITextBox*>(OFindUI("txtViewWidth"));
    ui_txtViewHeight = dynamic_cast<onut::UITextBox*>(OFindUI("txtViewHeight"));

    ui_propertiesNode = OFindUI("propertiesNode");
    ui_btnCreateNode = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateNode"));
    ui_txtSpriteName = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteName"));
    ui_txtSpriteType = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteType"));
    ui_chkNodeVisible = dynamic_cast<onut::UICheckBox*>(OFindUI("chkNodeVisible"));
    ui_txtSpriteX = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteX"));
    ui_txtSpriteY = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteY"));
    ui_txtSpriteScaleX = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteScaleX"));
    ui_txtSpriteScaleY = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteScaleY"));
    ui_txtSpriteScaleX->step = 0.01f;
    ui_txtSpriteScaleY->step = 0.01f;
    ui_txtSpriteAngle = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteAngle"));
    ui_txtSpriteAngle->step = 1.f;
    ui_colSpriteColor = dynamic_cast<onut::UIPanel*>(OFindUI("colSpriteColor"));
    ui_txtSpriteAlpha = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteAlpha"));
    ui_txtSpriteAlpha->min = 0.f;
    ui_txtSpriteAlpha->max = 100.f;

    ui_propertiesSprite = OFindUI("propertiesSprite");
    ui_btnCreateSprite = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateSprite"));
    ui_txtSpriteTexture = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteTexture"));
    ui_btnSpriteTextureBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnSpriteTextureBrowse"));
    ui_chkSpriteFlippedH = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSpriteFlippedH"));
    ui_chkSpriteFlippedV = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSpriteFlippedV"));
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

    ui_propertiesSpriteString = OFindUI("propertiesSpriteString");
    ui_btnCreateSpriteString = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateSpriteString"));
    ui_txtSpriteStringFont = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteStringFont"));
    ui_btnSpriteStringFontBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnSpriteStringFontBrowse"));
    ui_txtSpriteStringCaption = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteStringCaption"));
    ui_txtSpriteAlignX = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteAlignX"));
    ui_txtSpriteAlignY = dynamic_cast<onut::UITextBox*>(OFindUI("txtSpriteAlignY"));
    ui_txtSpriteAlignX->step = 0.01f;
    ui_txtSpriteAlignY->step = 0.01f;

    ui_propertiesEmitter = OFindUI("propertiesEmitter");
    ui_btnCreateEmitter = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateEmitter"));
    ui_txtEmitterFx = dynamic_cast<onut::UITextBox*>(OFindUI("txtEmitterFx"));
    ui_txtEmitterFxBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnEmitterFxBrowse"));
    ui_chkEmitterEmitWorld = dynamic_cast<onut::UICheckBox*>(OFindUI("chkEmitterEmitWorld"));
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

    ui_propertiesSoundEmitter = OFindUI("propertiesSoundEmitter");
    ui_btnCreateSoundEmitter = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateSoundEmitter"));
    ui_txtSoundEmitter = dynamic_cast<onut::UITextBox*>(OFindUI("txtSoundEmitter"));
    ui_btnSoundEmitterBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnSoundEmitterBrowse"));
    ui_chkSoundEmitterLoop = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSoundEmitterLoop"));
    ui_chkSoundEmitterPositionBased = dynamic_cast<onut::UICheckBox*>(OFindUI("chkSoundEmitterPositionBased"));
    ui_txtSoundEmitterVolume = dynamic_cast<onut::UITextBox*>(OFindUI("txtSoundEmitterVolume"));
    ui_txtSoundEmitterVolume->min = 0.f;
    ui_txtSoundEmitterVolume->max = 100.f;
    ui_txtSoundEmitterVolume->step = 1.f;
    ui_txtSoundEmitterBalance = dynamic_cast<onut::UITextBox*>(OFindUI("txtSoundEmitterBalance"));
    ui_txtSoundEmitterBalance->min = -100.f;
    ui_txtSoundEmitterBalance->max = 100.f;
    ui_txtSoundEmitterBalance->step = 1.f;
    ui_txtSoundEmitterPitch = dynamic_cast<onut::UITextBox*>(OFindUI("txtSoundEmitterPitch"));
    ui_txtSoundEmitterPitch->min = 10;
    ui_txtSoundEmitterPitch->max = 200;
    ui_txtSoundEmitterPitch->step = 1.f;

    ui_propertiesMusicEmitter = OFindUI("propertiesMusicEmitter");
    ui_btnCreateMusicEmitter = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateMusicEmitter"));
    ui_txtMusicEmitter = dynamic_cast<onut::UITextBox*>(OFindUI("txtMusicEmitter"));
    ui_btnMusicEmitterBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnMusicEmitterBrowse"));
    ui_chkMusicEmitterLoop = dynamic_cast<onut::UICheckBox*>(OFindUI("chkMusicEmitterLoop"));
    ui_txtMusicEmitterVolume = dynamic_cast<onut::UITextBox*>(OFindUI("txtMusicEmitterVolume"));
    ui_txtMusicEmitterVolume->min = 0.f;
    ui_txtMusicEmitterVolume->max = 100.f;
    ui_txtMusicEmitterVolume->step = 1.f;

    ui_propertiesVideo = OFindUI("propertiesVideo");
    ui_btnCreateVideo = dynamic_cast<onut::UIButton*>(OFindUI("btnCreateVideo"));
    ui_txtVideo = dynamic_cast<onut::UITextBox*>(OFindUI("txtVideo"));
    ui_btnVideoBrowse = dynamic_cast<onut::UIButton*>(OFindUI("btnVideoBrowse"));
    ui_chkVideoLoop = dynamic_cast<onut::UICheckBox*>(OFindUI("chkVideoLoop"));
    ui_txtVideoVolume = dynamic_cast<onut::UITextBox*>(OFindUI("txtVideoVolume"));
    ui_txtVideoVolume->min = 0.f;
    ui_txtVideoVolume->max = 100.f;
    ui_txtVideoVolume->step = 1.f;
    ui_txtVideoPlayRate = dynamic_cast<onut::UITextBox*>(OFindUI("txtVideoPlayRate"));
    ui_txtVideoPlayRate->min = 10;
    ui_txtVideoPlayRate->max = 200;
    ui_txtVideoPlayRate->step = 1.f;
    ui_txtVideoWidth = dynamic_cast<onut::UITextBox*>(OFindUI("txtVideoWidth"));
    ui_txtVideoHeight = dynamic_cast<onut::UITextBox*>(OFindUI("txtVideoHeight"));

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

                NodeState* stateBefore = new NodeState(pContainer);
                transformToParent(pTargetContainer->pNode, pContainer->pNode);
                NodeState* stateAfter = new NodeState(pContainer);

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
                }, [=]
                {
                }, [=]
                {
                    delete stateAfter;
                    delete stateBefore;
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

                auto stateBefore = new NodeState(pContainer);
                transformToParent(pTargetParent, pContainer->pNode);
                auto stateAfter = new NodeState(pContainer);

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
                }, [=]
                {
                }, [=]
                {
                    delete stateBefore;
                    delete stateAfter;
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

                auto stateBefore = new NodeState(pContainer);
                transformToParent(pTargetParent, pContainer->pNode);
                auto stateAfter = new NodeState(pContainer);

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
                }, [=]
                {
                }, [=]
                {
                    delete stateBefore;
                    delete stateAfter;
                }));
            }

            actionManager.doAction(pGroup);
        }
    };

    auto onStartSpinning = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        isSpinning = true;
        for (auto pContainer : selection)
        {
            pContainer->stateOnDown = NodeState(pContainer);
        }
    };
    auto onStopSpinning = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        isSpinning = false;
        for (auto pContainer : selection)
        {
            pContainer->stateOnDown.apply();
        }
        pControl->onTextChanged(pControl, event);
    };

    ui_txtViewWidth->onNumberSpinStart = onStartSpinning;
    ui_txtViewHeight->onNumberSpinStart = onStartSpinning;
    ui_txtSpriteX->onNumberSpinStart = onStartSpinning;
    ui_txtSpriteY->onNumberSpinStart = onStartSpinning;
    ui_txtSpriteScaleX->onNumberSpinStart = onStartSpinning;
    ui_txtSpriteScaleY->onNumberSpinStart = onStartSpinning;
    ui_txtSpriteAlignX->onNumberSpinStart = onStartSpinning;
    ui_txtSpriteAlignY->onNumberSpinStart = onStartSpinning;
    ui_txtSpriteAngle->onNumberSpinStart = onStartSpinning;
    ui_txtSpriteAlpha->onNumberSpinStart = onStartSpinning;
    ui_txtSoundEmitterVolume->onNumberSpinStart = onStartSpinning;
    ui_txtSoundEmitterBalance->onNumberSpinStart = onStartSpinning;
    ui_txtSoundEmitterPitch->onNumberSpinStart = onStartSpinning;
    ui_txtMusicEmitterVolume->onNumberSpinStart = onStartSpinning;
    ui_txtVideoVolume->onNumberSpinStart = onStartSpinning;
    ui_txtVideoPlayRate->onNumberSpinStart = onStartSpinning;
    ui_txtVideoWidth->onNumberSpinStart = onStartSpinning;
    ui_txtVideoHeight->onNumberSpinStart = onStartSpinning;

    ui_txtViewWidth->onNumberSpinEnd = onStopSpinning;
    ui_txtViewHeight->onNumberSpinEnd = onStopSpinning;
    ui_txtSpriteX->onNumberSpinEnd = onStopSpinning;
    ui_txtSpriteY->onNumberSpinEnd = onStopSpinning;
    ui_txtSpriteScaleX->onNumberSpinEnd = onStopSpinning;
    ui_txtSpriteScaleY->onNumberSpinEnd = onStopSpinning;
    ui_txtSpriteAlignX->onNumberSpinEnd = onStopSpinning;
    ui_txtSpriteAlignY->onNumberSpinEnd = onStopSpinning;
    ui_txtSpriteAngle->onNumberSpinEnd = onStopSpinning;
    ui_txtSpriteAlpha->onNumberSpinEnd = onStopSpinning;
    ui_txtSoundEmitterVolume->onNumberSpinEnd = onStopSpinning;
    ui_txtSoundEmitterBalance->onNumberSpinEnd = onStopSpinning;
    ui_txtSoundEmitterPitch->onNumberSpinEnd = onStopSpinning;
    ui_txtMusicEmitterVolume->onNumberSpinEnd = onStopSpinning;
    ui_txtVideoVolume->onNumberSpinEnd = onStopSpinning;
    ui_txtVideoPlayRate->onNumberSpinEnd = onStopSpinning;
    ui_txtVideoWidth->onNumberSpinEnd = onStopSpinning;
    ui_txtVideoHeight->onNumberSpinEnd = onStopSpinning;

    ui_txtViewWidth->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        auto oldSize = viewSize;
        auto newSize = Vector2((float)ui_txtViewWidth->getInt(), viewSize.y);
        if (isSpinning)
        {
            viewSize = newSize;
            return;
        }
        actionManager.doAction(new onut::Action("Change View Width",
            [=]
        {
            viewSize = newSize;
            markModified();
        }, [=]
        {
            viewSize = oldSize;
            markModified();
        }));
        updateProperties();
    };
    ui_txtViewHeight->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        auto oldSize = viewSize;
        auto newSize = Vector2(viewSize.x, (float)ui_txtViewHeight->getInt());
        if (isSpinning)
        {
            viewSize = newSize;
            return;
        }
        actionManager.doAction(new onut::Action("Change View Height",
            [=]
        {
            viewSize = newSize;
            markModified();
        }, [=]
        {
            viewSize = oldSize;
            markModified();
        }));
        updateProperties();
    };
    ui_txtSpriteName->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Name", [](std::shared_ptr<NodeContainer> pContainer)
        {
            pContainer->pNode->SetName(ui_txtSpriteName->textComponent.text);
        });
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
    ui_txtSpriteStringCaption->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Caption", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSpriteString = dynamic_cast<seed::SpriteString*>(pContainer->pNode);
            if (pSpriteString)
            {
                pSpriteString->SetCaption(ui_txtSpriteStringCaption->textComponent.text);
            }
        });
    };
    ui_txtSpriteX->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Position X", [](std::shared_ptr<NodeContainer> pContainer)
        {
            pContainer->pNode->SetPosition(Vector2(ui_txtSpriteX->getFloat(), pContainer->pNode->GetPosition().y));
        });
    };
    ui_txtSpriteY->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Position Y", [](std::shared_ptr<NodeContainer> pContainer)
        {
            pContainer->pNode->SetPosition(Vector2(pContainer->pNode->GetPosition().x, ui_txtSpriteY->getFloat()));
        });
    };
    ui_txtVideoWidth->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Video Width", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
            if (pVideo)
            {
                pVideo->SetDimensions(Vector2(ui_txtVideoWidth->getFloat(), pVideo->GetDimensions().y));
            }
        });
    };
    ui_txtVideoHeight->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Video Height", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
            if (pVideo)
            {
                pVideo->SetDimensions(Vector2(pVideo->GetDimensions().x, ui_txtVideoHeight->getFloat()));
            }
        });
    };
    ui_txtSpriteY->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Position Y", [](std::shared_ptr<NodeContainer> pContainer)
        {
            pContainer->pNode->SetPosition(Vector2(pContainer->pNode->GetPosition().x, ui_txtSpriteY->getFloat()));
        });
    };
    ui_txtSpriteScaleX->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Scale X", [](std::shared_ptr<NodeContainer> pContainer)
        {
            pContainer->pNode->SetScale(Vector2(ui_txtSpriteScaleX->getFloat(), pContainer->pNode->GetScale().y));
        });
    };
    ui_txtSpriteScaleY->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Scale Y", [](std::shared_ptr<NodeContainer> pContainer)
        {
            pContainer->pNode->SetScale(Vector2(pContainer->pNode->GetScale().x, ui_txtSpriteScaleY->getFloat()));
        });
    };
    ui_txtSpriteAlignX->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Align X", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetAlign(Vector2(ui_txtSpriteAlignX->getFloat(), pSprite->GetAlign().y));
            }
        });
    };
    ui_txtSpriteAlignY->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Align Y", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetAlign(Vector2(pSprite->GetAlign().x, ui_txtSpriteAlignY->getFloat()));
            }
        });
    };
    ui_txtSpriteAngle->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Angle", [](std::shared_ptr<NodeContainer> pContainer)
        {
            pContainer->pNode->SetAngle(ui_txtSpriteAngle->getFloat());
        });
    };
    ui_chkNodeVisible->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        changeSpriteProperty("Change Visibility", [](std::shared_ptr<NodeContainer> pContainer)
        {
            pContainer->pNode->SetVisible(ui_chkNodeVisible->getIsChecked());
        });
    };
    ui_chkEmitterEmitWorld->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        changeSpriteProperty("Change Emit World", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pEmitter = dynamic_cast<seed::Emitter*>(pContainer->pNode);
            if (pEmitter)
            {
                pEmitter->SetEmitWorld(ui_chkEmitterEmitWorld->getIsChecked());
            }
        });
    };
    ui_chkSoundEmitterLoop->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        changeSpriteProperty("Change Loop", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
            if (pSoundEmitter)
            {
                pSoundEmitter->SetLoops(ui_chkSoundEmitterLoop->getIsChecked());
            }
        });
    };
    ui_chkMusicEmitterLoop->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        changeSpriteProperty("Change Loop", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
            if (pMusicEmitter)
            {
                pMusicEmitter->SetLoops(ui_chkMusicEmitterLoop->getIsChecked());
            }
        });
    };
    ui_chkVideoLoop->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        changeSpriteProperty("Change Loop", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
            if (pVideo)
            {
                pVideo->SetLoops(ui_chkVideoLoop->getIsChecked());
            }
        });
    };
    ui_chkSoundEmitterPositionBased->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        changeSpriteProperty("Change Position Based", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
            if (pSoundEmitter)
            {
                pSoundEmitter->SetPositionBasedVolume(ui_chkSoundEmitterPositionBased->getIsChecked());
                pSoundEmitter->SetPositionBasedBalance(ui_chkSoundEmitterPositionBased->getIsChecked());
            }
        });
    };
    ui_chkSpriteFlippedH->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        changeSpriteProperty("Change Flip H", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetFlipped(ui_chkSpriteFlippedH->getIsChecked(), pSprite->GetFlippedV());
            }
        });
    };
    ui_chkSpriteFlippedV->onCheckChanged = [](onut::UICheckBox* pControl, const onut::UICheckEvent& event)
    {
        changeSpriteProperty("Change Flip V", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSprite = dynamic_cast<seed::Sprite*>(pContainer->pNode);
            if (pSprite)
            {
                pSprite->SetFlipped(pSprite->GetFlippedH(), ui_chkSpriteFlippedV->getIsChecked());
            }
        });
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
    ui_colSpriteColor->onClick = [=](onut::UIControl* pControl, const onut::UIMouseEvent& evt)
    {
        static COLORREF g_acrCustClr[16]; // array of custom colors

        CHOOSECOLOR colorChooser = {0};
        DWORD rgbCurrent; // initial color selection
        rgbCurrent = (DWORD)ui_colSpriteColor->color.packed;
        rgbCurrent = ((rgbCurrent >> 24) & 0x000000ff) | ((rgbCurrent >> 8) & 0x0000ff00) | ((rgbCurrent << 8) & 0x00ff0000);
        colorChooser.lStructSize = sizeof(colorChooser);
        colorChooser.hwndOwner = OWindow->getHandle();
        colorChooser.lpCustColors = (LPDWORD)g_acrCustClr;
        colorChooser.rgbResult = rgbCurrent;
        colorChooser.Flags = CC_FULLOPEN | CC_RGBINIT;
        if (ChooseColor(&colorChooser) == TRUE)
        {
            onut::sUIColor color;
            rgbCurrent = colorChooser.rgbResult;
            color.packed = ((rgbCurrent << 24) & 0xff000000) | ((rgbCurrent << 8) & 0x00ff0000) | ((rgbCurrent >> 8) & 0x0000ff00) | 0x000000ff;
            color.unpack();
            ui_colSpriteColor->color = color;
            changeSpriteProperty("Change Color", [color](std::shared_ptr<NodeContainer> pContainer)
            {
                auto colorBefore = pContainer->pNode->GetColor();
                pContainer->pNode->SetColor(Color(color.r, color.g, color.b, colorBefore.w));
            });
        }
    };
    ui_txtSpriteAlpha->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Alpha", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto alpha = ui_txtSpriteAlpha->getFloat() / 100.f;
            pContainer->pNode->SetColor(Color(pContainer->pNode->GetColor().x, pContainer->pNode->GetColor().y, pContainer->pNode->GetColor().z, alpha));
        });
    };
    ui_txtSoundEmitterVolume->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Volume", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
            if (pSoundEmitter)
            {
                auto volume = ui_txtSoundEmitterVolume->getFloat() / 100.f;
                pSoundEmitter->SetVolume(volume);
            }
        });
    };
    ui_txtMusicEmitterVolume->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Volume", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pMusicEmitter = dynamic_cast<seed::MusicEmitter*>(pContainer->pNode);
            if (pMusicEmitter)
            {
                auto volume = ui_txtMusicEmitterVolume->getFloat() / 100.f;
                pMusicEmitter->SetVolume(volume);
            }
        });
    };
    ui_txtVideoVolume->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Volume", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
            if (pVideo)
            {
                auto volume = ui_txtVideoVolume->getFloat() / 100.f;
                pVideo->SetVolume(volume);
            }
        });
    };
    ui_txtSoundEmitterBalance->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Balance", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
            if (pSoundEmitter)
            {
                auto balance = ui_txtSoundEmitterBalance->getFloat() / 100.f;
                pSoundEmitter->SetBalance(balance);
            }
        });
    };
    ui_txtSoundEmitterPitch->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Pitch", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pSoundEmitter = dynamic_cast<seed::SoundEmitter*>(pContainer->pNode);
            if (pSoundEmitter)
            {
                auto pitch = ui_txtSoundEmitterPitch->getFloat() / 100.f;
                pSoundEmitter->SetPitch(pitch);
            }
        });
    };
    ui_txtVideoPlayRate->onTextChanged = [](onut::UITextBox* pControl, const onut::UITextBoxEvent& event)
    {
        changeSpriteProperty("Change Play Rate", [](std::shared_ptr<NodeContainer> pContainer)
        {
            auto pVideo = dynamic_cast<seed::Video*>(pContainer->pNode);
            if (pVideo)
            {
                auto playRate = ui_txtVideoPlayRate->getFloat() / 100.f;
                pVideo->SetPlayRate(playRate);
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

    ui_btnCreateSpriteString->onClick = [](onut::UIControl* pControl, const onut::UIMouseEvent& event)
    {
        createSpriteString("segeo12.fnt");
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
                auto transform = pContainer->stateOnDown.parentTransform;
                auto worldPos = Vector2::Transform(pContainer->stateOnDown.position, transform);
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

            auto invTransform = pContainer->stateOnDown.transform.Invert();
            invTransform._41 = 0;
            invTransform._42 = 0;

            auto localMouseDiff = Vector2::Transform(mouseDiff, invTransform);
            auto localScaleDiff = handle.transformDirection * localMouseDiff;
            Vector2 newScale;
            if (OPressed(OINPUT_LSHIFT))
            {
                localScaleDiff.x = localScaleDiff.y = std::max<>(localScaleDiff.x, localScaleDiff.y);
                newScale = pContainer->stateOnDown.scale + localScaleDiff / size * 2.f * pContainer->stateOnDown.scale;
                auto ratioOnDown = pContainer->stateOnDown.scale.x / pContainer->stateOnDown.scale.y;
                newScale.y = newScale.x / ratioOnDown;
            }
            else
            {
                newScale = pContainer->stateOnDown.scale + localScaleDiff / size * 2.f * pContainer->stateOnDown.scale;
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
                    auto screenPosition = Vector2::Transform(pContainer->stateOnDown.transform.Translation(), viewTransform);
                    auto centerVect = screenPosition - selectionCenter;
                    centerVect = Vector2::Transform(centerVect, Matrix::CreateRotationZ(DirectX::XMConvertToRadians(angleDiff)));
                    screenPosition = centerVect + selectionCenter;
                    auto viewPosition = Vector2::Transform(screenPosition, invViewTransform);
                    auto invParentTransform = pContainer->stateOnDown.parentTransform.Invert();
                    auto localPosition = Vector2::Transform(viewPosition, invParentTransform);
                    pContainer->pNode->SetPosition(localPosition);
                    pContainer->pNode->SetAngle(pContainer->stateOnDown.angle + angleDiff);
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
                pContainer->pNode->SetAngle(pContainer->stateOnDown.angle + angleDiff);
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
                pContainer->stateOnDown.apply();
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
