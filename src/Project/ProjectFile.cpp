// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "ProjectFile.h"

#include <Helper/Messaging.h>
#include <Helper/SelectionHelper.h>
#include <ctools/FileHelper.h>

ProjectFile::ProjectFile() = default;

ProjectFile::ProjectFile(const std::string& vFilePathName)
{
	m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
	if (ps.isOk)
	{
		m_ProjectFilePath = ps.path;
	}
}

ProjectFile::~ProjectFile() = default;

void ProjectFile::Clear()
{
	m_ProjectFilePathName.clear();
	m_ProjectFilePath.clear();
	m_MergedFontPrefix.clear();
	m_Fonts.clear();
	m_ShowRangeColoring = false;
	m_RangeColoringHash = ImVec4(10, 15, 35, 0.5f);
	m_Preview_Glyph_CountX = 20;
	m_Preview_Glyph_Width = 50;
	m_CardGlyphHeightInPixel = 40U; // ine item height in card
	m_CardCountRowsMax = 20U; // after this max, new columns
	m_SelectedFont = nullptr;
	m_CountSelectedGlyphs = 0; // for all fonts
	m_IsLoaded = false;
	m_IsThereAnyNotSavedChanged = false;
	m_GenMode = GENERATOR_MODE_CURRENT_HEADER_CARD |
		GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES;
	m_SourcePane_ShowGlyphTooltip = true;
	m_FinalPane_ShowGlyphTooltip = true;
	m_CurrentPane_ShowGlyphTooltip = true;

	Messaging::Instance()->Clear();
}

void ProjectFile::New()
{
	Clear();
	m_IsLoaded = true;
	m_NeverSaved = true;
}

void ProjectFile::New(const std::string& vFilePathName)
{
	Clear();
	m_ProjectFilePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
	if (ps.isOk)
	{
		m_ProjectFilePath = ps.path;
	}
	m_IsLoaded = true;
	SetProjectChange(false);
}

bool ProjectFile::Load()
{
	return LoadAs(m_ProjectFilePathName);
}

// ils wanted to not pass the adress for re open case
// elwse, the clear will set vFilePathName to empty because with re open, target m_ProjectFilePathName
bool ProjectFile::LoadAs(const std::string vFilePathName)  
{
	Clear();
	std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	if (LoadConfigFile(filePathName))
	{
		m_ProjectFilePathName = filePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(m_ProjectFilePathName);
		if (ps.isOk)
		{
			m_ProjectFilePath = ps.path;
		}
		if (m_SelectedFont)
		{
			m_LastGeneratedFileName = m_SelectedFont->m_FontFileName;
		}
		m_IsLoaded = true;
		SetProjectChange(false);
	}
	else
	{
		Clear();
	}

	return m_IsLoaded;
}

bool ProjectFile::Save()
{
	if (m_NeverSaved) 
		return false;

	if (SaveConfigFile(m_ProjectFilePathName))
	{
		SetProjectChange(false);
		return true;
	}

	return false;
}

bool ProjectFile::SaveAs(const std::string& vFilePathName)
{
	std::string filePathName = FileHelper::Instance()->SimplifyFilePath(vFilePathName);
	auto ps = FileHelper::Instance()->ParsePathFileName(filePathName);
	if (ps.isOk)
	{
		m_ProjectFilePathName = FileHelper::Instance()->ComposePath(ps.path, ps.name, "ifs");
		m_ProjectFilePath = ps.path;
		m_NeverSaved = false;
		return Save();
	}
	return false;
}

bool ProjectFile::IsLoaded() const
{
	return m_IsLoaded;
}

bool ProjectFile::IsNeverSaved() const
{
	return m_NeverSaved;
}

bool ProjectFile::IsThereAnyNotSavedChanged() const
{
	return m_IsThereAnyNotSavedChanged;
}

void ProjectFile::SetProjectChange(bool vChange)
{
	m_IsThereAnyNotSavedChanged = vChange;
}

void ProjectFile::UpdateCountSelectedGlyphs()
{
	m_CountSelectedGlyphs = 0;
	m_CountFontWithSelectedGlyphs = 0;

	for (auto &it : m_Fonts)
	{
		m_CountSelectedGlyphs += it.second.m_SelectedGlyphs.size();
		if (!it.second.m_SelectedGlyphs.empty())
		{
			m_CountFontWithSelectedGlyphs++;
		}
	}

	SelectionHelper::Instance()->AnalyseSourceSelection(this);
}

bool ProjectFile::IsRangeColoringShown() const
{
	return m_ShowRangeColoring || SelectionHelper::Instance()->IsSelectionType(GlyphSelectionTypeFlags::GLYPH_SELECTION_TYPE_BY_RANGE);
}

std::string ProjectFile::GetAbsolutePath(const std::string& vFilePathName) const
{
	std::string res = vFilePathName;

	if (!vFilePathName.empty())
	{
		if (!FileHelper::Instance()->IsAbsolutePath(vFilePathName)) // relative
		{
			res = FileHelper::Instance()->SimplifyFilePath(
				m_ProjectFilePath + FileHelper::Instance()->m_SlashType + vFilePathName);
		}
	}

	return res;
}

std::string ProjectFile::GetRelativePath(const std::string& vFilePathName) const
{
	std::string res = vFilePathName;

	if (!vFilePathName.empty())
	{
		res = FileHelper::Instance()->GetRelativePathToPath(vFilePathName, m_ProjectFilePath);
	}

	return res;
}

std::string ProjectFile::getXml(const std::string& vOffset)
{
	std::string str;

	str += vOffset + "<project>\n";

	if (!m_Fonts.empty())
	{
		for (auto &it : m_Fonts)
		{
			str += it.second.getXml(vOffset + "\t");
		}
	}

	str += vOffset + "\t<rangecoloring show=\"" + 
		(m_ShowRangeColoring ? "true" : "false") + "\" hash=\"" + 
		ct::fvec4(m_RangeColoringHash).string() + "\"/>\n";

	str += vOffset + "\t<previewglyphcount>" + ct::toStr(m_Preview_Glyph_CountX) + "</previewglyphcount>\n";
	str += vOffset + "\t<previewglyphwidth>" + ct::toStr(m_Preview_Glyph_Width) + "</previewglyphwidth>\n";
	str += vOffset + "\t<mergedfontprefix>" + m_MergedFontPrefix + "</mergedfontprefix>\n";

	str += vOffset + "\t<curglyphtooltip>" + (m_CurrentPane_ShowGlyphTooltip ? "true" : "false") + "</curglyphtooltip>\n";
	str += vOffset + "\t<srcglyphtooltip>" + (m_SourcePane_ShowGlyphTooltip ? "true" : "false") +"</srcglyphtooltip>\n";
	str += vOffset + "\t<dstglyphtooltip>" + (m_FinalPane_ShowGlyphTooltip ? "true" : "false") +"</dstglyphtooltip>\n";

	str += vOffset + "\t<glyphpreviewscale>" + ct::toStr(m_GlyphPreview_Scale) + "</glyphpreviewscale>\n";
	str += vOffset + "\t<glyphpreviewshowcontrollines>" + (m_GlyphPreview_ShowControlLines ? "true" : "false") + "</glyphpreviewshowcontrollines>\n";
	str += vOffset + "\t<glyphpreviewquadbeziercounsegment>" + ct::toStr(m_GlyphPreview_QuadBezierCountSegments) + "</glyphpreviewquadbeziercounsegment>\n";

	str += vOffset + "\t<genmode>" + ct::toStr(m_GenMode) + "</genmode>\n";
	str += vOffset + "\t<fonttomergein>" + m_FontToMergeIn + "</fonttomergein>\n";
	
	str += vOffset + "\t<glyphdisplaytuningmode>" + ct::toStr(m_GlyphDisplayTuningMode) + "</glyphdisplaytuningmode>\n";
	str += vOffset + "\t<sourcefontpaneflags>" + ct::toStr(m_SourceFontPaneFlags) + "</sourcefontpaneflags>\n";
	
	str += vOffset + "\t<cardglyhpheight>" + ct::toStr(m_CardGlyphHeightInPixel) + "</cardglyhpheight>\n";
	str += vOffset + "\t<cardcountrowsmax>" + ct::toStr(m_CardCountRowsMax) + "</cardcountrowsmax>\n";

	str += vOffset + "\t<lastgeneratedpath>" + m_LastGeneratedPath + "</lastgeneratedpath>\n";
	str += vOffset + "\t<lastgeneratedfilename>" + m_LastGeneratedFileName + "</lastgeneratedfilename>\n";

	str += vOffset + "</project>\n";

	return str;
}

bool ProjectFile::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strName == "project")
	{

	}

	if (strParentName == "project")
	{
		if (strName == "font")
		{
			FontInfos f;
			f.setFromXml(vElem, vParent);
			if (!f.m_FontFileName.empty())
			{
				m_Fonts[f.m_FontFileName] = f;
			}
		}
		else if (strName == "rangecoloring")
		{
			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "show")
					m_ShowRangeColoring = ct::ivariant(attValue).GetB();
				else if (attName == "hash")
					m_RangeColoringHash = ct::toImVec4(ct::fvariant(attValue).GetV4());
			}
		}
		else if (strName == "previewglyphcount")
			m_Preview_Glyph_CountX = ct::ivariant(strValue).GetI();
		else if (strName == "previewglyphwidth")
			m_Preview_Glyph_Width = ct::fvariant(strValue).GetF();
		else if (strName == "mergedfontprefix")
			m_MergedFontPrefix = strValue;
		else if (strName == "genmode")
			m_GenMode = (GenModeFlags)ct::ivariant(strValue).GetI();
		else if (strName == "fonttomergein")
			m_FontToMergeIn = strValue;
		else if (strName == "curglyphtooltip")
			m_CurrentPane_ShowGlyphTooltip = ct::ivariant(strValue).GetB();
		else if (strName == "srcglyphtooltip")
			m_SourcePane_ShowGlyphTooltip = ct::ivariant(strValue).GetB();
		else if (strName == "dstglyphtooltip")
			m_FinalPane_ShowGlyphTooltip = ct::ivariant(strValue).GetB();
		else if (strName == "glyphpreviewscale")
			m_GlyphPreview_Scale = ct::fvariant(strValue).GetF();
		else if (strName == "glyphpreviewshowcontrollines")
			m_GlyphPreview_ShowControlLines = ct::ivariant(strValue).GetB();
		else if (strName == "glyphpreviewquadbeziercounsegment")
			m_GlyphPreview_QuadBezierCountSegments = ct::ivariant(strValue).GetI();
		else if (strName == "glyphdisplaytuningmode")
			m_GlyphDisplayTuningMode = (GlyphDisplayTuningModeFlags)ct::ivariant(strValue).GetI();
		else if (strName == "sourcefontpaneflags")
			m_SourceFontPaneFlags = (SourceFontPaneFlags)ct::ivariant(strValue).GetI();
		else if (strName == "cardglyhpheight")
			m_CardGlyphHeightInPixel = ct::uvariant(strValue).GetU();
		else if (strName == "cardcountrowsmax")
			m_CardCountRowsMax = ct::uvariant(strValue).GetU();
		else if (strName == "lastgeneratedpath")
			m_LastGeneratedPath = strValue;
		else if (strName == "lastgeneratedfilename")
			m_LastGeneratedFileName = strValue;
	}

	return true;
}

ImVec4 ProjectFile::GetColorFromInteger(uint32_t vInteger) const
{
	ImVec4 res;

	if (IsLoaded())
	{
		res.x = sinf(m_RangeColoringHash.x * (float)vInteger) * 0.5f + 0.5f;
		res.y = sinf(m_RangeColoringHash.y * (float)vInteger) * 0.5f + 0.5f;
		res.z = sinf(m_RangeColoringHash.z * (float)vInteger) * 0.5f + 0.5f;
		res.w = m_RangeColoringHash.w;
	}

	return res;
}

void ProjectFile::AddGenMode(GenModeFlags vFlags)
{
	m_GenMode |= vFlags;
}

void ProjectFile::RemoveGenMode(GenModeFlags vFlags)
{
	m_GenMode &= ~vFlags;
}

GenModeFlags ProjectFile::GetGenMode() const
{
	return m_GenMode;
}

 bool ProjectFile::IsGenMode(GenModeFlags vFlags) const
{
	return (m_GenMode & vFlags);
}

