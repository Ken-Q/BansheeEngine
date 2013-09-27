#include "BsGUIListBox.h"
#include "BsImageSprite.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsSpriteTexture.h"
#include "BsTextSprite.h"
#include "BsGUILayoutOptions.h"
#include "BsGUIMouseEvent.h"
#include "BsGUIManager.h"
#include "BsGUIHelper.h"
#include "BsGUIDropDownBoxManager.h"
#include "CmTexture.h"

using namespace CamelotFramework;

namespace BansheeEngine
{
	const String& GUIListBox::getGUITypeName()
	{
		static String name = "ListBox";
		return name;
	}

	GUIListBox::GUIListBox(GUIWidget& parent, const GUIElementStyle* style, const Vector<WString>::type& elements, const GUILayoutOptions& layoutOptions)
		:GUIElement(parent, style, layoutOptions), mElements(elements), mNumImageRenderElements(0), mSelectedIdx(0), mIsListBoxOpen(false)
	{
		mImageSprite = cm_new<ImageSprite, PoolAlloc>();
		mTextSprite = cm_new<TextSprite, PoolAlloc>();

		mImageDesc.texture = mStyle->normal.texture;

		if(mImageDesc.texture != nullptr)
		{
			mImageDesc.width = mImageDesc.texture->getTexture()->getWidth();
			mImageDesc.height = mImageDesc.texture->getTexture()->getHeight();
		}

		mImageDesc.borderLeft = mStyle->border.left;
		mImageDesc.borderRight = mStyle->border.right;
		mImageDesc.borderTop = mStyle->border.top;
		mImageDesc.borderBottom = mStyle->border.bottom;
	}

	GUIListBox::~GUIListBox()
	{
		cm_delete<PoolAlloc>(mTextSprite);
		cm_delete<PoolAlloc>(mImageSprite);

		closeListBox();
	}

	GUIListBox* GUIListBox::create(GUIWidget& parent, const Vector<WString>::type& elements, const GUIElementStyle* style)
	{
		if(style == nullptr)
		{
			const GUISkin& skin = parent.getSkin();
			style = skin.getStyle(getGUITypeName());
		}

		return new (cm_alloc<GUIListBox, PoolAlloc>()) GUIListBox(parent, style, elements, getDefaultLayoutOptions(style));
	}

	GUIListBox* GUIListBox::create(GUIWidget& parent, const Vector<WString>::type& elements, const GUILayoutOptions& layoutOptions, const GUIElementStyle* style)
	{
		if(style == nullptr)
		{
			const GUISkin& skin = parent.getSkin();
			style = skin.getStyle(getGUITypeName());
		}

		return new (cm_alloc<GUIListBox, PoolAlloc>()) GUIListBox(parent, style, elements, layoutOptions);
	}

	UINT32 GUIListBox::getNumRenderElements() const
	{
		UINT32 numElements = mImageSprite->getNumRenderElements();
		numElements += mTextSprite->getNumRenderElements();

		return numElements;
	}

	const HMaterial& GUIListBox::getMaterial(UINT32 renderElementIdx) const
	{
		if(renderElementIdx >= mNumImageRenderElements)
			return mTextSprite->getMaterial(mNumImageRenderElements - renderElementIdx);
		else
			return mImageSprite->getMaterial(renderElementIdx);
	}

	UINT32 GUIListBox::getNumQuads(UINT32 renderElementIdx) const
	{
		UINT32 numQuads = 0;
		if(renderElementIdx >= mNumImageRenderElements)
			numQuads = mTextSprite->getNumQuads(mNumImageRenderElements - renderElementIdx);
		else
			numQuads = mImageSprite->getNumQuads(renderElementIdx);

		return numQuads;
	}

	void GUIListBox::updateRenderElementsInternal()
	{		
		mImageDesc.width = mWidth;
		mImageDesc.height = mHeight;

		mImageSprite->update(mImageDesc);
		mNumImageRenderElements = mImageSprite->getNumRenderElements();

		mTextSprite->update(getTextDesc());

		GUIElement::updateRenderElementsInternal();
	}

	void GUIListBox::updateClippedBounds()
	{
		mClippedBounds = mImageSprite->getBounds(mOffset, mClipRect);
	}

	Int2 GUIListBox::_getOptimalSize() const
	{
		UINT32 imageWidth = 0;
		UINT32 imageHeight = 0;
		if(mImageDesc.texture != nullptr)
		{
			imageWidth = mImageDesc.texture->getTexture()->getWidth();
			imageHeight = mImageDesc.texture->getTexture()->getHeight();
		}

		Int2 contentSize = GUIHelper::calcOptimalContentsSize(mElements[mSelectedIdx], *mStyle, _getLayoutOptions());
		UINT32 contentWidth = std::max(imageWidth, (UINT32)contentSize.x);
		UINT32 contentHeight = std::max(imageHeight, (UINT32)contentSize.y);

		return Int2(contentWidth, contentHeight);
	}

	UINT32 GUIListBox::_getRenderElementDepth(UINT32 renderElementIdx) const
	{
		if(renderElementIdx >= mNumImageRenderElements)
			return _getDepth();
		else
			return _getDepth() + 1;
	}

	void GUIListBox::fillBuffer(UINT8* vertices, UINT8* uv, UINT32* indices, UINT32 startingQuad, UINT32 maxNumQuads, 
		UINT32 vertexStride, UINT32 indexStride, UINT32 renderElementIdx) const
	{
		if(renderElementIdx >= mNumImageRenderElements)
		{
			Rect textBounds = getContentBounds();
			Int2 offset(textBounds.x, textBounds.y);
			Rect textClipRect = getContentClipRect();

			mTextSprite->fillBuffer(vertices, uv, indices, startingQuad, maxNumQuads, 
				vertexStride, indexStride, mNumImageRenderElements - renderElementIdx, offset, textClipRect);
		}
		else
		{
			mImageSprite->fillBuffer(vertices, uv, indices, startingQuad, maxNumQuads, 
				vertexStride, indexStride, renderElementIdx, mOffset, mClipRect);
		}
	}

	bool GUIListBox::mouseEvent(const GUIMouseEvent& ev)
	{
		if(ev.getType() == GUIMouseEventType::MouseOver)
		{
			mImageDesc.texture = mStyle->hover.texture;
			markContentAsDirty();

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseOut)
		{
			mImageDesc.texture = mStyle->normal.texture;
			markContentAsDirty();

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseDown)
		{
			mImageDesc.texture = mStyle->active.texture;
			markContentAsDirty();

			openListBox();

			return true;
		}
		else if(ev.getType() == GUIMouseEventType::MouseUp)
		{
			mImageDesc.texture = mStyle->hover.texture;
			markContentAsDirty();

			return true;
		}

		return false;
	}

	void GUIListBox::elementSelected(CM::UINT32 idx)
	{
		if(!onSelectionChanged.empty())
			onSelectionChanged(idx);

		mSelectedIdx = idx;

		closeListBox();

		markContentAsDirty();
	}

	void GUIListBox::openListBox()
	{
		closeListBox();

		Vector<GUIDropDownData>::type dropDownData;
		UINT32 i = 0;
		for(auto& elem : mElements)
		{
			dropDownData.push_back(GUIDropDownData::button(elem, boost::bind(&GUIListBox::elementSelected, this, i)));
			i++;
		}

		GUIWidget& widget = _getParentWidget();
		GUIDropDownAreaPlacement placement = GUIDropDownAreaPlacement::aroundBoundsHorz(getBounds());

		GameObjectHandle<GUIDropDownBox> dropDownBox = GUIDropDownBoxManager::instance().openDropDownBox(widget.getTarget(), widget.getOwnerWindow(), 
			placement, dropDownData, widget.getSkin(), GUIDropDownType::MenuBar, boost::bind(&GUIListBox::onListBoxClosed, this));

		GUIManager::instance().enableSelectiveInput(boost::bind(&GUIListBox::closeListBox, this));
		GUIManager::instance().addSelectiveInputWidget(dropDownBox.get());
		GUIManager::instance().addSelectiveInputElement(this);

		mIsListBoxOpen = true;
	}

	void GUIListBox::closeListBox()
	{
		if(mIsListBoxOpen)
		{
			GUIDropDownBoxManager::instance().closeDropDownBox();
			GUIManager::instance().disableSelectiveInput();
			mIsListBoxOpen = false;
		}
	}

	void GUIListBox::onListBoxClosed()
	{
		GUIManager::instance().disableSelectiveInput();
		mIsListBoxOpen = false;
	}

	TEXT_SPRITE_DESC GUIListBox::getTextDesc() const
	{
		TEXT_SPRITE_DESC textDesc;
		textDesc.text = mElements[mSelectedIdx];
		textDesc.font = mStyle->font;
		textDesc.fontSize = mStyle->fontSize;

		Rect textBounds = getContentBounds();

		textDesc.width = textBounds.width;
		textDesc.height = textBounds.height;
		textDesc.horzAlign = mStyle->textHorzAlign;
		textDesc.vertAlign = mStyle->textVertAlign;

		return textDesc;
	}
}