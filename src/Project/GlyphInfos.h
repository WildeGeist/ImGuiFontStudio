/*
 * Copyright 2020 Stephane Cuillerdier (aka Aiekick)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <ctools/ConfigAbstract.h>
#include <tinyxml2/tinyxml2.h>
#include <imgui/imgui.h>
#include <string>
#include <memory>
#include <ctools/cTools.h>
#include <sfntly/table/truetype/glyph_table.h>

#define GLYPH_EDIT_CONTROl_WIDTH 180.0f

class ProjectFile;
class GlyphDisplayHelper
{
public:
	static int CalcGlyphsCountAndSize(			/* return new glyph count x				*/
		ProjectFile* vProjectFile,				/* project file for save some vars		*/
		ImVec2* vCellSize,						/* cell size							*/
		ImVec2* vGlyphSize,						/* glyph size (cell - paddings)			*/
		bool vGlyphEdited = false,				/* in edition							*/
		bool vForceEditMode = false,			/* edition forced						*/
		bool vForceEditModeOneColumn = false);	/* edition in one column				*/
};

class FontInfos;
class GlyphInfos;
class SimpleGlyph_Solo
{
public:
	bool isValid = false;

public:
	std::vector<std::vector<ct::ivec2>> coords;
	std::vector<std::vector<bool>> onCurve;
	ct::ivec4 rc;
	ct::fvec2 m_Translation; // translation in first
	ct::fvec2 m_Scale = 1.0f; // scale in second

public:
	void Clear();
	void LoadSimpleGlyph(sfntly::GlyphTable::SimpleGlyph *vGlyph);
	int GetCountContours() const;
	ct::ivec2 GetCoords(int32_t vContour, int32_t vPoint);
	bool IsOnCurve(int32_t vContour, int32_t vPoint);
	ct::ivec2 Scale(ct::ivec2 p, double scale) const;
	ct::ivec2 GetCoords(int32_t vContour, int32_t vPoint, double scale);
	void ClearTransform();

public: // ImGui
	void DrawCurves(
		float vGlobalScale,
		std::shared_ptr<FontInfos> vFontInfos,
		std::shared_ptr<GlyphInfos> vGlyphInfos,
		int vMaxContour, int vQuadBezierCountSegments, 
		bool vShowControlLines, bool vShowGlyphLegends);
};

class GlyphInfos
{
public:
	static std::shared_ptr<GlyphInfos> Create(
		std::shared_ptr<FontInfos> vFontInfos,
		ImFontGlyph vGlyph, std::string vOldName,
		std::string vNewName, uint32_t vNewCodePoint = 0,
		ImVec2 vTranslation = ImVec2(0, 0));
	// 0 => none, 1 => left pressed, 2 => right pressed
	static int DrawGlyphButton(
		int& vWidgetPushId, // by adress because we want modify it
		ProjectFile* vProjectFile, ImFont* vFont,
		bool* vSelected, ImVec2 vGlyphSize, const ImFontGlyph* vGlyph,
		ImVec2 vTranslation = ImVec2(0, 0), ImVec2 vScale = ImVec2(1, 1),
		int frame_padding = -1, float vRectThickNess = 0.0f, ImVec4 vRectColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	static void RenderGlyph(
		ImFont* vFont, ImDrawList* vDrawList, 
		float vGlyphHeight, ImVec2 vMin, ImVec2 vMax, ImVec2 vOffset,
		ImU32 vCol, 
		ImWchar vGlyphCodePoint, 
		ImVec2 vTranslation, ImVec2 vScale, 
		bool vZoomed);

private:
	std::weak_ptr<FontInfos> fontInfos;

public:
	ImFontGlyph glyph{};
	int glyphIndex = 0;
	std::string oldHeaderName;
	std::string newHeaderName;
	uint32_t newCodePoint = 0;
	SimpleGlyph_Solo simpleGlyph;
	ct::fvec2 m_Translation = 0.0f;
	ct::fvec2 m_Scale = 1.0f;

	// filled during generation only
	// not to use outside of generation
	ct::ivec4 m_FontBoundingBox;
	int m_FontAscent = 0;
	int m_FontDescent = 0;
	
public: // for interaction only
	bool m_editingName = false;
	bool m_editingCodePoint = false;
	int editCodePoint = 0;
	std::string editHeaderName;

public:
	GlyphInfos();
	GlyphInfos(
		std::shared_ptr<FontInfos> vFontInfos,
		ImFontGlyph vGlyph, std::string vOldName,
		std::string vNewName, uint32_t vNewCodePoint, 
		ImVec2 vTranslation);
	~GlyphInfos();

	std::shared_ptr<FontInfos> GetFontInfos();
	void SetFontInfos(std::shared_ptr<FontInfos> vFontInfos);
};
