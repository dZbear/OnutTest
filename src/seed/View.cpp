#include "View.h"
#include "Sprite.h"
#include "onut.h"

#define VIEW_DEFAULT_SPRITE_COUNT 64


namespace seed
{
	View::View()
		: m_spritePool(sizeof(Sprite), VIEW_DEFAULT_SPRITE_COUNT)
	{
	}

	View::~View()
	{

	}

	void View::Show()
	{
		OnShow();
	}

	void View::Hide()
	{
        // free all sprites
        DeleteSprites();
		OnHide();
	}

	void View::Update()
	{
		OnUpdate();
	}

	void View::Render()
	{
		// render sprites
		for (Sprite* s : m_sprites)
		{
			if (!s->GetParent())
			{
				// render this sprite if it doesn't have a parent
				s->Render();
			}
		}
		OnRender();
	}

	Sprite* View::AddSprite(const string& in_textureName, int in_zIndex)
	{
		OTexture* texture = OGetTexture(in_textureName.c_str());
		if (!texture)
		{
			OLogE("Invalid texture name specified to View::AddSprite : " + in_textureName);
			return nullptr;
		}

		Sprite* newSprite = m_spritePool.alloc<Sprite>();
		newSprite->SetZindex(in_zIndex);
		newSprite->SetTexture(texture);
		if (in_zIndex == INT_MAX)
		{
			m_sprites.push_back(newSprite);
		}
		else
		{
			InsertSprite(newSprite, in_zIndex);
		}
		return newSprite;
	}

	void View::DeleteSprite(Sprite* in_sprite)
	{
		for (size_t i = 0, size = m_sprites.size(); i < size; ++i)
		{
			if (m_sprites[i] == in_sprite)
			{
				if (in_sprite->GetParent())
				{
					in_sprite->GetParent()->Detach(in_sprite);
				}
				m_sprites.erase(m_sprites.begin() + i);
				m_spritePool.dealloc(in_sprite);
				return;
			}
		}
	}

	void View::InsertSprite(Sprite* in_sprite, int in_zIndex)
	{
		for (size_t i = 0, size = m_sprites.size(); i < size; ++i )
		{
			if (m_sprites[i]->GetZindex() > in_zIndex)
			{
				// let's insert before this one
				m_sprites.insert(m_sprites.begin() + i, in_sprite);
				return;
			}
		}

		// if we're here it means we didnt find any suitable place for it, just insert at the end
		m_sprites.push_back(in_sprite);
	}

    void View::DeleteSprites()
    {
        for (size_t i = 0, size = m_sprites.size(); i < size; ++i)
        {
            m_spritePool.dealloc(m_sprites[i]);
        }
        m_sprites.clear();
    }
}





