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

#include "Generator.h"

#include "imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"

// header
#include <ios>
#include "sfntly/font.h"
#include "sfntly/port/file_input_stream.h"
#include "sfntly/tag.h"

// cpp
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <cTools.h>
#include <FileHelper.h>
#include "Helper/FontHelper.h"
#include "Helper/Messaging.h"

#include "Panes/SourceFontPane.h"
#include "Project/FontInfos.h"
#include "Project/ProjectFile.h"
#include "Helper/Messaging.h"

Generator::Generator()
{
	
}

Generator::~Generator()
{
	
}

void Generator::Generate(
	const std::string& vFilePath,
	const std::string& vFileName,
	ProjectFile* vProjectFile)
{
	if (vProjectFile)
	{
		if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_CPP))
		{
			if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_CURRENT))
			{
				std::string filePathName = vFilePath + "/" + vFileName;
				GenerateCpp_One(filePathName, vProjectFile, vProjectFile->m_CurrentFont, vProjectFile->m_GenMode);
			}
			else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_BATCH))
			{
				for (auto &font : vProjectFile->m_Fonts)
				{
					auto ps = FileHelper::Instance()->ParsePathFileName(font.second.m_FontFileName);
					if (ps.isOk)
					{
						std::string filePathName = vFilePath + "/" + ps.name + "." + ps.ext;
						GenerateCpp_One(filePathName, vProjectFile, &font.second, vProjectFile->m_GenMode);
					}
				}
			}
			else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_MERGED))
			{
				std::string filePathName = vFilePath + "/" + vFileName;
				GenerateCpp_Merged(filePathName, vProjectFile, vProjectFile->GetGenMode());
			}
		}
		else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_FONT))
		{
			if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_CURRENT))
			{
				std::string filePathName = vFilePath + "/" + vFileName;
				GenerateFontFile_One(filePathName, vProjectFile, vProjectFile->m_CurrentFont, vProjectFile->m_GenMode);
#ifdef _DEBUG
				SourceFontPane::Instance()->OpenFont(vProjectFile, filePathName, false); // directly load the generated font file
#endif
			}
			else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_BATCH))
			{
				for (auto &font : vProjectFile->m_Fonts)
				{
					auto ps = FileHelper::Instance()->ParsePathFileName(font.second.m_FontFileName);
					if (ps.isOk)
					{
						std::string filePathName = vFilePath + "/" + ps.name + "." + ps.ext;
						GenerateFontFile_One(filePathName, vProjectFile, &font.second, vProjectFile->m_GenMode);
#ifdef _DEBUG
						SourceFontPane::Instance()->OpenFont(vProjectFile, filePathName, false); // directly load the generated font file
#endif
					}
				}
			}
			else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_MERGED))
			{
				std::string filePathName = vFilePath + "/" + vFileName;
				GenerateFontFile_Merged(filePathName, vProjectFile, vProjectFile->m_GenMode);
#ifdef _DEBUG
				SourceFontPane::Instance()->OpenFont(vProjectFile, filePathName, false); // directly load the generated font file
#endif
			}
		}
		else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_HEADER))
		{
			if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_CURRENT))
			{
				std::string filePathName = vFilePath + "/" + vFileName;
				GenerateHeader_One(filePathName, vProjectFile->m_CurrentFont, vProjectFile->m_GenMode);
			}
			else if (vProjectFile->IsGenMode(GenModeFlags::GENERATOR_MODE_BATCH))
			{
				for (auto &font : vProjectFile->m_Fonts)
				{
					auto ps = FileHelper::Instance()->ParsePathFileName(font.second.m_FontFileName);
					if (ps.isOk)
					{
						std::string filePathName = vFilePath + "/" + ps.name + ".h";
						GenerateHeader_One(filePathName, &font.second, vProjectFile->m_GenMode);
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// HEADER GENERATION ////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

/* 03/03/2020 22h20 it work like a charm for the two modes */
/*
Generate Header file with glyphs code like in https://github.com/juliettef/IconFontCppHeaders
two modes :
- if not glyph were selected => will generate header for all font glyphs
- if glyph were selected => will generate header only for these glyphs
todo : handle error code to a messagebox to display in app
*/
void Generator::GenerateHeader_One(
	const std::string& vFilePathName, 
	FontInfos *vFontInfos, 
	const GenModeFlags& vFlags,
	std::string vFontBufferName, // for header generation wehn using a cpp bytes array instead of a file
	size_t vFontBufferSize) // for header generation wehn using a cpp bytes array instead of a file
{
	if (!vFilePathName.empty() && vFontInfos)
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			filePathName = ps.path + FileHelper::Instance()->m_SlashType + ps.name + ".h";
		}

		std::string header;
		header += "//Header Generated with https://github.com/aiekick/ImGuiFontStudio\n";
		header += "//Based on https://github.com/juliettef/IconFontCppHeaders\n";
		header += "\n#pragma once\n\n";

		std::string prefix = "";
		if (!vFontInfos->m_FontPrefix.empty())
			prefix = "_" + vFontInfos->m_FontPrefix;
		if (vFontBufferSize > 0)
		{
			header += "#define FONT_ICON_BUFFER_NAME" + prefix + " " + vFontBufferName + "\n";
			header += "#define FONT_ICON_BUFFER_SIZE" + prefix + " 0x" + ct::toHexStr(vFontBufferSize) + "\n\n";
		}
		else
		{
			header += "#define FONT_ICON_FILE_NAME" + prefix + " \"" + vFontInfos->m_FontFileName + "\"\n\n";
		}
		
		ImWchar minCodePoint = 65535;
		ImWchar maxCodePoint = 0;
		std::string glyphs;

		if (vFlags & GenModeFlags::GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_NAMES)
		{
			std::map<std::string, ImWchar> glyphNames;

			if (vFontInfos->m_SelectedGlyphs.empty()) // no glyph selected so generate for whole font
			{
				for (auto &it : vFontInfos->m_GlyphCodePointNames)
				{
					glyphNames[it.second] = it.first;
				}
			}
			else
			{
				for (auto &it : vFontInfos->m_SelectedGlyphs)
				{
					glyphNames[it.second.newHeaderName] = it.second.newCodePoint;
				}
			}
			
			for (const auto &it : glyphNames)
			{
				std::string glyphName = it.first;
				ct::replaceString(glyphName, " ", "_");
				ct::replaceString(glyphName, "-", "_");
				
				ImWchar codePoint = it.second;
				minCodePoint = ct::mini(minCodePoint, codePoint);
				maxCodePoint = ct::maxi(maxCodePoint, codePoint);

				// by ex .notdef will become DOT_notdef
				// because a define with '.' is a problem for the compiler
				ct::replaceString(glyphName, ".", "DOT_");

				for (auto & c : glyphName) 
					c = (char)toupper((int)c);

				glyphs += "#define ICON" + prefix + "_" + glyphName + " u8\"\\u" + ct::toHexStr(codePoint) + "\"\n";
			}
		}
		else if (vFlags & GenModeFlags::GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_CODEPOINT)
		{
			std::map<ImWchar, std::string> glyphCodePoints;

			if (vFontInfos->m_SelectedGlyphs.empty()) // no glyph selected so generate for whole font
			{
				for (auto &it : vFontInfos->m_GlyphCodePointNames)
				{
					glyphCodePoints[it.first] = it.second;
				}
			}
			else
			{
				for (auto &it : vFontInfos->m_SelectedGlyphs)
				{
					glyphCodePoints[it.second.newCodePoint] = it.second.newHeaderName;
				}
			}

			for (const auto &it : glyphCodePoints)
			{
				ImWchar codePoint = it.first;
				minCodePoint = ct::mini(minCodePoint, codePoint);
				maxCodePoint = ct::maxi(maxCodePoint, codePoint);

				std::string glyphName = it.second;
				ct::replaceString(glyphName, " ", "_");
				ct::replaceString(glyphName, "-", "_");

				// by ex .notdef will become DOT_notdef
				// because a define with '.' is a problem for the compiler
				ct::replaceString(glyphName, ".", "DOT_");

				for (auto & c : glyphName)
					c = (char)toupper((int)c);

				glyphs += "#define ICON" + prefix + "_" + glyphName + " u8\"\\u" + ct::toHexStr(codePoint) + "\"\n";
			}
		}

		// code point range
		header += "#define ICON_MIN" + prefix + " 0x" + ct::toHexStr(minCodePoint) + "\n";
		header += "#define ICON_MAX" + prefix + " 0x" + ct::toHexStr(maxCodePoint) + "\n\n";

		ct::replaceString(filePathName, "-", "_");

		FileHelper::Instance()->SaveStringToFile(header + glyphs, filePathName);
		FileHelper::Instance()->OpenFile(filePathName);
	}
}

void Generator::GenerateHeader_Merged(
	const std::string& vFilePathName,
	ProjectFile* vProjectFile,
	const GenModeFlags& vFlags,
	std::string vFontBufferName, // for header generation wehn using a cpp bytes array instead of a file
	size_t vFontBufferSize) // for header generation wehn using a cpp bytes array instead of a file
{
	if (vProjectFile &&
		!vFilePathName.empty() &&
		!vProjectFile->m_Fonts.empty())
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			filePathName = ps.path + FileHelper::Instance()->m_SlashType + ps.name + ".h";
		}

		std::string header;
		header += "//Header Generated with https://github.com/aiekick/ImGuiFontStudio\n";
		header += "//Based on https://github.com/juliettef/IconFontCppHeaders\n";
		header += "\n#pragma once\n\n";

		std::string prefix = "";
		if (!vProjectFile->m_MergedFontPrefix.empty())
			prefix = "_" + vProjectFile->m_MergedFontPrefix;
		if (vFontBufferSize > 0)
		{
			header += "#define FONT_ICON_BUFFER_NAME" + prefix + " " + vFontBufferName + "\n";
			header += "#define FONT_ICON_BUFFER_SIZE" + prefix + " 0x" + ct::toHexStr(vFontBufferSize) + "\n\n";
		}
		else
		{
			header += "#define FONT_ICON_FILE_NAME" + prefix + " \"" + ps.name + "." + ps.ext + "\"\n\n";
		}

		ImWchar minCodePoint = 65535;
		ImWchar maxCodePoint = 0;
		std::string glyphs;

		if (vFlags & GenModeFlags::GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_NAMES)
		{
			std::map<std::string, ImWchar> glyphNames;

			// we take only selected glyphs of all fonts
			for (const auto &font : vProjectFile->m_Fonts)
			{
				for (const auto &glyph : font.second.m_SelectedGlyphs)
				{
					glyphNames[glyph.second.newHeaderName] = glyph.second.newCodePoint;
				}
			}

			for (const auto &it : glyphNames)
			{
				std::string glyphName = it.first;
				ct::replaceString(glyphName, " ", "_");
				ct::replaceString(glyphName, "-", "_");

				ImWchar codePoint = it.second;
				minCodePoint = ct::mini(minCodePoint, codePoint);
				maxCodePoint = ct::maxi(maxCodePoint, codePoint);

				// by ex .notdef will become DOT_notdef
				// because a define with '.' is a problem for the compiler
				ct::replaceString(glyphName, ".", "DOT_");

				for (auto & c : glyphName)
					c = (char)toupper((int)c);

				glyphs += "#define ICON" + prefix + "_" + glyphName + " u8\"\\u" + ct::toHexStr(codePoint) + "\"\n";
			}
		}
		else if (vFlags & GenModeFlags::GENERATOR_MODE_HEADER_SETTINGS_ORDER_BY_CODEPOINT)
		{
			std::map<ImWchar, std::string> glyphCodePoints;

			// we take only selected glyphs of all fonts
			for (const auto &font : vProjectFile->m_Fonts)
			{
				for (const auto &glyph : font.second.m_SelectedGlyphs)
				{
					glyphCodePoints[glyph.second.newCodePoint] = glyph.second.newHeaderName;
				}
			}

			for (const auto &it : glyphCodePoints)
			{
				ImWchar codePoint = it.first;
				minCodePoint = ct::mini(minCodePoint, codePoint);
				maxCodePoint = ct::maxi(maxCodePoint, codePoint);

				std::string glyphName = it.second;
				ct::replaceString(glyphName, " ", "_");
				ct::replaceString(glyphName, "-", "_");

				// by ex .notdef will become DOT_notdef
				// because a define with '.' is a problem for the compiler
				ct::replaceString(glyphName, ".", "DOT_");

				for (auto & c : glyphName)
					c = (char)toupper((int)c);

				glyphs += "#define ICON" + prefix + "_" + glyphName + " u8\"\\u" + ct::toHexStr(codePoint) + "\"\n";
			}
		}

		// code point range
		header += "#define ICON_MIN" + prefix + " 0x" + ct::toHexStr(minCodePoint) + "\n";
		header += "#define ICON_MAX" + prefix + " 0x" + ct::toHexStr(maxCodePoint) + "\n\n";

		ct::replaceString(filePathName, "-", "_");

		FileHelper::Instance()->SaveStringToFile(header + glyphs, filePathName);
		FileHelper::Instance()->OpenFile(filePathName);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// FONT GENERATION //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

/* 03/03/2020 22h41 it work like a charm (Yihaaaa!!) */
/*
Generate Font File with selected Glyphs 
if vGenerateHeader is true, will Generate also a Header file with glyphs 
   code like in https://github.com/juliettef/IconFontCppHeaders
todo : handle error code to a messagebox to display in app
*/
void Generator::GenerateFontFile_One(
	const std::string& vFilePathName, 
	ProjectFile* vProjectFile,
	FontInfos *vFontInfos,
	const GenModeFlags& vFlags)
{
	if (vProjectFile && !vFilePathName.empty() && vFontInfos)
	{
		FontHelper fontHelper;

		bool res = false;

		std::map<int32_t, std::string> newHeaderNames;
		std::map<int32_t, int32_t> newCodePoints;
		std::map<CodePoint, GlyphInfos> newGlyphInfos;
		for (const auto &glyph : vFontInfos->m_SelectedGlyphs)
		{
			newHeaderNames[glyph.first] = glyph.second.newHeaderName;
			newCodePoints[glyph.first] = glyph.second.newCodePoint;
			newGlyphInfos[glyph.first] = glyph.second;
		}

		if (!newHeaderNames.empty() && !newCodePoints.empty())
		{
			std::string absPath = vProjectFile->GetAbsolutePath(vFontInfos->m_FontFilePathName);
			res = fontHelper.OpenFontFile(absPath, newHeaderNames, newCodePoints, newGlyphInfos, true);
		}

		if (res)
		{
			/*	std::set<int32_t> table_blacklist;
				table_blacklist.insert(sfntly::Tag::DSIG);
				table_blacklist.insert(sfntly::Tag::GDEF);
				table_blacklist.insert(sfntly::Tag::GPOS);
				table_blacklist.insert(sfntly::Tag::GSUB);
				table_blacklist.insert(sfntly::Tag::kern);
				table_blacklist.insert(sfntly::Tag::hdmx);
				table_blacklist.insert(sfntly::Tag::vmtx);
				table_blacklist.insert(sfntly::Tag::VDMX);
				table_blacklist.insert(sfntly::Tag::LTSH);
				table_blacklist.insert(sfntly::Tag::vhea);
				table_blacklist.insert(sfntly::Tag::morx);
				not used for the moment, we keep only head table
			*/
			
			std::string filePathName = vFilePathName;
			auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
			if (ps.isOk)
			{
				filePathName = ps.path + FileHelper::Instance()->m_SlashType + ps.name + ".ttf";
				if (filePathName[0] == FileHelper::Instance()->m_SlashType[0]) 
					filePathName = filePathName.substr(1);

				if (fontHelper.GenerateFontFile(filePathName, vFlags & GenModeFlags::GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES))
				{
					if (vFlags & GenModeFlags::GENERATOR_MODE_HEADER)
					{
						GenerateHeader_One(filePathName, vFontInfos, vFlags);
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, 0, 0, "Cannot create font file.\n");
					return;
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, 0, 0, "FilePathName is wrong : %s", vFilePathName.c_str());
				return;
			}
			
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0,
				"Could not open font file %s.\n", 
				vProjectFile->m_CurrentFont->m_FontFilePathName.c_str());
			return;
		}
	}
}

void Generator::GenerateFontFile_Merged(
	const std::string& vFilePathName, 
	ProjectFile* vProjectFile,
	const GenModeFlags& vFlags)
{
	if (vProjectFile && vProjectFile->IsLoaded() &&
		!vFilePathName.empty() &&
		!vProjectFile->m_Fonts.empty() &&
		!vProjectFile->m_FontToMergeIn.empty())
	{
		FontHelper fontHelper;

		bool res = true;

		ct::ivec2 baseSize = 0;
		ct::ivec4 baseFontBoundingBox;
		int baseFontAscent = 0;
		int baseFontDescent = 0;

		// abse infos for merge all toher fonts in this one
		for (auto &it : vProjectFile->m_Fonts)
		{
			if (vProjectFile->m_FontToMergeIn == it.second.m_FontFileName)
			{
				baseFontBoundingBox = it.second.m_BoundingBox;
				baseFontAscent = it.second.m_Ascent;
				baseFontDescent = it.second.m_Descent;
				baseSize = it.second.m_BoundingBox.zw() - it.second.m_BoundingBox.xy();
			}
		}

		for (auto &it : vProjectFile->m_Fonts)
		{
			bool scaleChanged = false;
			ct::dvec2 scale = 1.0;
			if (vProjectFile->m_FontToMergeIn != it.second.m_FontFileName)
			{
				scaleChanged = true;
				ct::ivec2 newSize = it.second.m_BoundingBox.zw() - it.second.m_BoundingBox.xy();
				scale.x = (double)baseSize.x / (double)newSize.x;
				scale.y = (double)baseSize.y / (double)newSize.y;
				double v = ct::mini(scale.x, scale.y);
				scale.x = v; // same value for keep glyph ratio
				scale.y = v; // same value for keep glyph ratio
			}

			std::map<int32_t, std::string> newHeaderNames;
			std::map<int32_t, int32_t> newCodePoints;
			std::map<CodePoint, GlyphInfos> newGlyphInfos;
			for (const auto &glyph : it.second.m_SelectedGlyphs)
			{
				GlyphInfos gInfos = glyph.second;

				newHeaderNames[glyph.first] = gInfos.newHeaderName;
				newCodePoints[glyph.first] = gInfos.newCodePoint;

				if (scaleChanged)
				{
					gInfos.simpleGlyph.isValid = true;
					gInfos.simpleGlyph.m_Scale = scale;

					gInfos.m_FontBoundingBox = baseFontBoundingBox;
					gInfos.m_FontAscent = baseFontAscent;
					gInfos.m_FontDescent = baseFontDescent;

				}
				
				newGlyphInfos[glyph.first] = gInfos;
			}

			if (!newHeaderNames.empty())
			{
				std::string absPath = vProjectFile->GetAbsolutePath(it.second.m_FontFilePathName);
				res &= fontHelper.OpenFontFile(absPath, newHeaderNames, newCodePoints, newGlyphInfos, !scaleChanged);
			}
			else
			{
				res = false;
			}
		}
		
		if (res)
		{
			std::set<int32_t> table_blacklist;
			/*
				table_blacklist.insert(sfntly::Tag::DSIG);
				table_blacklist.insert(sfntly::Tag::GDEF);
				table_blacklist.insert(sfntly::Tag::GPOS);
				table_blacklist.insert(sfntly::Tag::GSUB);
				table_blacklist.insert(sfntly::Tag::kern);
				table_blacklist.insert(sfntly::Tag::hdmx);
				table_blacklist.insert(sfntly::Tag::vmtx);
				table_blacklist.insert(sfntly::Tag::VDMX);
				table_blacklist.insert(sfntly::Tag::LTSH);
				table_blacklist.insert(sfntly::Tag::vhea);
				table_blacklist.insert(sfntly::Tag::morx);
				not used for the moment, we keep only head table
			*/

			std::string filePathName = vFilePathName;
			auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
			if (ps.isOk)
			{
				filePathName = ps.path + FileHelper::Instance()->m_SlashType + ps.name + ".ttf";
				if (filePathName[0] == FileHelper::Instance()->m_SlashType[0]) 
					filePathName = filePathName.substr(1);

				if (fontHelper.GenerateFontFile(vFilePathName, vFlags & GenModeFlags::GENERATOR_MODE_FONT_SETTINGS_USE_POST_TABLES))
				{
					if (vFlags & GenModeFlags::GENERATOR_MODE_HEADER)
					{
						GenerateHeader_Merged(filePathName, vProjectFile, vFlags);
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, 0, 0, "Cannot create font file.\n");
					return;
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, 0, 0, "FilePathName is wrong : %s", vFilePathName.c_str());
				return;
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0,
				"Could not open font file %s.\n",
				vProjectFile->m_CurrentFont->m_FontFilePathName.c_str());
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// CPP GENERATION ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// imported from https://github.com/ocornut/imgui/blob/master/misc/fonts/binary_to_compressed_c.cpp
typedef unsigned int stb_uint;
typedef unsigned char stb_uchar;
stb_uint stb_compress(stb_uchar *out, stb_uchar *in, stb_uint len);

char Encode85Byte(unsigned int x)
{
	x = (x % 85) + 35;
	return (char)((x >= (uint32_t)'\\') ? x + 1 : x);
}

/* 03/03/2020 23h38 it work like a charm (Wouhoooooo!!)
will generate cpp fille with/without header
two modes :
- no glyph selected => export whole font file to cpp (and header is asked)
- some glyohs selectef => export glyph selection in a new temporary font file, then exported in cpp (and header if asked)
*/
void Generator::GenerateCpp_One(
	const std::string& vFilePathName, 
	ProjectFile* vProjectFile,
	FontInfos *vFontInfos,
	const GenModeFlags& vFlags)
{
	if (vProjectFile && !vFilePathName.empty() && vFontInfos)
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::string res;

			bool generateTemporaryFontFile = false;
			if (vFontInfos->m_SelectedGlyphs.empty()) // export whole font file to cpp (and header is asked)
			{
				filePathName = vProjectFile->GetAbsolutePath(vFontInfos->m_FontFilePathName);
				// todo: il faut gerer le cas ou on va exporter tout les fichier sans selction
				// ici on ne fait que l'actif, c'est aps ce qu'on veut
			}
			else // export glyph selection in a new temporary font file, then exported in cpp (and header if asked)
			{
				// generate temporary font file first
				generateTemporaryFontFile = true;
				filePathName = ps.path + "\\temporary_" + ps.name + ".ttf";
				GenerateFontFile_One(filePathName, vProjectFile, vFontInfos,
					(GenModeFlags)(vFlags & ~GenModeFlags::GENERATOR_MODE_HEADER)); // no header to generate
			}

			if (FileHelper::Instance()->IsFileExist(filePathName))
			{
				std::string bufferName;
				size_t bufferSize = 0;
				res = get_Compressed_Base85_BytesArray(
					filePathName,
					vFontInfos->m_FontPrefix,
					&bufferName,
					&bufferSize);

				if (generateTemporaryFontFile)
				{
					// we have the result, if empty or not we need to destroy the temporary font file
					FileHelper::Instance()->DestroyFile(filePathName);
				}

				// if ok, serialization
				if (!res.empty() && !bufferName.empty() && bufferSize > 0)
				{
					filePathName = ps.path + FileHelper::Instance()->m_SlashType + ps.name + ".cpp";

					if (vFlags & GenModeFlags::GENERATOR_MODE_HEADER)
					{
						res = "#include \"" + ps.name + ".h\"\n\n" + res;
						std::string prefix = "";
						if (!vFontInfos->m_FontPrefix.empty())
							prefix = "_" + vFontInfos->m_FontPrefix;
						prefix = "FONT_ICON_BUFFER_NAME" + prefix;
						ct::replaceString(res, vFontInfos->m_FontPrefix + "_compressed_data_base85", prefix);

						GenerateHeader_One(filePathName, vFontInfos, vFlags, bufferName, bufferSize);
					}

					FileHelper::Instance()->SaveStringToFile(res, filePathName);
					FileHelper::Instance()->OpenFile(filePathName);
				}
				else
				{
					Messaging::Instance()->AddError(true, 0, 0,
						"Error opening or reading file %s", filePathName.c_str());
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, 0, 0,
					"Cant found file %s", filePathName.c_str());
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0,
				"Bad FilepathName %s", filePathName.c_str());
		}
	}
}

void Generator::GenerateCpp_Merged(
	const std::string& vFilePathName, 
	ProjectFile* vProjectFile,
	const GenModeFlags& vFlags)
{
	if (vProjectFile &&	!vFilePathName.empty())
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::string res;

			filePathName = ps.path + "\\temporary_" + ps.name + ".ttf";
			GenerateFontFile_Merged(filePathName, vProjectFile,
				(GenModeFlags)(vFlags & ~GenModeFlags::GENERATOR_MODE_HEADER)); // no header to generate

			if (FileHelper::Instance()->IsFileExist(filePathName))
			{
				std::string bufferName;
				size_t bufferSize = 0;
				res = get_Compressed_Base85_BytesArray(
					filePathName,
					vProjectFile->m_MergedFontPrefix,
					&bufferName,
					&bufferSize);

				// we have the result, if empty or not we need to destroy the temporary font file
				FileHelper::Instance()->DestroyFile(filePathName);

				// if ok, serialization
				if (!res.empty() && !bufferName.empty() && bufferSize > 0)
				{
					filePathName = ps.path + FileHelper::Instance()->m_SlashType + ps.name + ".cpp";

					if (vFlags & GenModeFlags::GENERATOR_MODE_HEADER)
					{
						res = "#include \"" + ps.name + ".h\"\n\n" + res;
						std::string prefix = "";
						if (!vProjectFile->m_MergedFontPrefix.empty())
							prefix = "_" + vProjectFile->m_MergedFontPrefix;
						prefix = "FONT_ICON_BUFFER_NAME" + prefix;
						ct::replaceString(res, vProjectFile->m_MergedFontPrefix + "_compressed_data_base85", prefix);

						GenerateHeader_Merged(filePathName, vProjectFile, vFlags, bufferName, bufferSize);
					}

					FileHelper::Instance()->SaveStringToFile(res, filePathName);
					FileHelper::Instance()->OpenFile(filePathName);
				}
				else
				{
					Messaging::Instance()->AddError(true, 0, 0,
						"Error opening or reading file %s", filePathName.c_str());
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, 0, 0,
					"Cant found file %s", filePathName.c_str());
			}
		}
		else
		{
			Messaging::Instance()->AddError(true, 0, 0,
				"Bad FilepathName %s", filePathName.c_str());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// UTILS ////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// based on https://github.com/ocornut/imgui/tree/master/misc/fonts/binary_to_compressed_c.cpp
// and modified for export bytes Array (for avoid compiler limitation with char array of more than 2^16 (65536) chars)
std::string Generator::get_Compressed_Base85_BytesArray(
	const std::string& vFilePathName, 
	const std::string& vPrefix, 
	std::string *vBufferName, 
	size_t *vBufferSize)
{
	std::string res;

	// Read file
#ifdef MSVC
	FILE *f = 0;
	errno_t err = fopen_s(&f, vFilePathName.c_str(), "rb");
	if (err) return res;
#else
	FILE* f = fopen(vFilePathName.c_str(), "rb");
	if (!f) return res;
#endif

	int data_sz;
	if (fseek(f, 0, SEEK_END) || (data_sz = (int)ftell(f)) == -1 || fseek(f, 0, SEEK_SET)) { fclose(f); return res; }
	char* data = new char[data_sz + 4];
	if (fread(data, 1, data_sz, f) != (size_t)data_sz) { fclose(f); delete[] data; return res; }
	memset((void*)(((char*)data) + data_sz), 0, 4);
	fclose(f);

	// Compress
	int maxlen = data_sz + 512 + (data_sz >> 2) + sizeof(int); // total guess
	char* compressed = new char[maxlen];
	int compressed_sz = stb_compress((stb_uchar*)compressed, (stb_uchar*)data, data_sz);
	memset(compressed + compressed_sz, 0, maxlen - compressed_sz);

	// Output as Base85 encoded
	size_t bufferSize = (int)((compressed_sz + 3) / 4) * 5;
	bool generateByteArray = (bufferSize >= 65536);
	if (vBufferSize)
		*vBufferSize = bufferSize; // export buffer size
	std::string bufferName = vPrefix + "_compressed_data_base85";
	if (vBufferName)
		*vBufferName = bufferName;
	res += "static const char " + bufferName + "[" + ct::toStr(bufferSize) + "+1] =";
	if (generateByteArray)
	{
		res += "{\n";
	}
	else
	{
		res += "\n    \"";
	}
	
	std::string content;

	char prev_c = 0;
	for (int src_i = 0; src_i < compressed_sz; src_i += 4)
	{
		// This is made a little more complicated by the fact that ??X sequences are interpreted as trigraphs by old C/C++ compilers. So we need to escape pairs of ??.
		unsigned int d = *(unsigned int*)(compressed + src_i);
		for (unsigned int n5 = 0; n5 < 5; n5++, d /= 85)
		{
			char c = Encode85Byte(d);
			if (generateByteArray)
			{
				char buf[20];
				int len = snprintf(buf, 19, "%02x", c);
				if (len)
				{
					content += "0x" + std::string(buf, len) + ", ";
				}
			}
			else
			{
				content += (c == '?' && prev_c == '?') ? "\\" + ct::toStr(c) : ct::toStr(c);
			}
			
			prev_c = c;
		}
		if ((src_i % 112) == 112 - 4)
		{
			if (generateByteArray)
			{
				content += "\n";
			}
			else
			{
				content += "\"\n    \"";
			}
		}
	}
	
	if (generateByteArray)
	{
		content = content.substr(0, content.size() - 2);
		res += content + "};\n";
	}
	else
	{
		res += content + "\";\n\n";
	}

	// Cleanup
	delete[] data;
	delete[] compressed;

	return res;
}

// imported from https://github.com/ocornut/imgui/blob/master/misc/fonts/binary_to_compressed_c.cpp
// stb_compress* from stb.h - definition
//////////////////// compressor ///////////////////////

static stb_uint stb_adler32(stb_uint adler32, stb_uchar *buffer, stb_uint buflen)
{
	const unsigned long ADLER_MOD = 65521;
	unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
	unsigned long blocklen, i;

	blocklen = buflen % 5552;
	while (buflen) {
		for (i = 0; i + 7 < blocklen; i += 8) {
			s1 += buffer[0], s2 += s1;
			s1 += buffer[1], s2 += s1;
			s1 += buffer[2], s2 += s1;
			s1 += buffer[3], s2 += s1;
			s1 += buffer[4], s2 += s1;
			s1 += buffer[5], s2 += s1;
			s1 += buffer[6], s2 += s1;
			s1 += buffer[7], s2 += s1;

			buffer += 8;
		}

		for (; i < blocklen; ++i)
			s1 += *buffer++, s2 += s1;

		s1 %= ADLER_MOD, s2 %= ADLER_MOD;
		buflen -= blocklen;
		blocklen = 5552;
	}
	return (s2 << 16) + s1;
}

static unsigned int stb_matchlen(stb_uchar *m1, stb_uchar *m2, stb_uint maxlen)
{
	stb_uint i;
	for (i = 0; i < maxlen; ++i)
		if (m1[i] != m2[i]) return i;
	return i;
}

// simple implementation that just takes the source data in a big block

static stb_uchar *stb__out;
static FILE      *stb__outfile;
static stb_uint   stb__outbytes;

static void stb__write(unsigned char v)
{
	fputc(v, stb__outfile);
	++stb__outbytes;
}

//#define stb_out(v)    (stb__out ? *stb__out++ = (stb_uchar) (v) : stb__write((stb_uchar) (v)))
#define stb_out(v)    do { if (stb__out) *stb__out++ = (stb_uchar) (v); else stb__write((stb_uchar) (v)); } while (0)

static void stb_out2(stb_uint v) { stb_out(v >> 8); stb_out(v); }
static void stb_out3(stb_uint v) { stb_out(v >> 16); stb_out(v >> 8); stb_out(v); }
static void stb_out4(stb_uint v) { stb_out(v >> 24); stb_out(v >> 16); stb_out(v >> 8); stb_out(v); }

static void outliterals(stb_uchar *in, int numlit)
{
	while (numlit > 65536) {
		outliterals(in, 65536);
		in += 65536;
		numlit -= 65536;
	}

	if (numlit == 0);
	else if (numlit <= 32)    stb_out(0x000020 + numlit - 1);
	else if (numlit <= 2048)    stb_out2(0x000800 + numlit - 1);
	else /*  numlit <= 65536) */ stb_out3(0x070000 + numlit - 1);

	if (stb__out) {
		memcpy(stb__out, in, numlit);
		stb__out += numlit;
	}
	else
		fwrite(in, 1, numlit, stb__outfile);
}

static int stb__window = 0x40000; // 256K

static int stb_not_crap(int best, int dist)
{
	return   ((best > 2 && dist <= 0x00100)
		|| (best > 5 && dist <= 0x04000)
		|| (best > 7 && dist <= 0x80000));
}

static  stb_uint stb__hashsize = 32768;

// note that you can play with the hashing functions all you
// want without needing to change the decompressor
#define stb__hc(q,h,c)      (((h) << 7) + ((h) >> 25) + q[c])
#define stb__hc2(q,h,c,d)   (((h) << 14) + ((h) >> 18) + (q[c] << 7) + q[d])
#define stb__hc3(q,c,d,e)   ((q[c] << 14) + (q[d] << 7) + q[e])

static unsigned int stb__running_adler;

static int stb_compress_chunk(stb_uchar *history,
	stb_uchar *start,
	stb_uchar *end,
	int length,
	int *pending_literals,
	stb_uchar **chash,
	stb_uint mask)
{
	(void)history;
	int window = stb__window;
	stb_uint match_max;
	stb_uchar *lit_start = start - *pending_literals;
	stb_uchar *q = start;

#define STB__SCRAMBLE(h)   (((h) + ((h) >> 16)) & mask)

	// stop short of the end so we don't scan off the end doing
	// the hashing; this means we won't compress the last few bytes
	// unless they were part of something longer
	while (q < start + length && q + 12 < end) {
		int m;
		stb_uint h1, h2, h3, h4, h;
		stb_uchar *t;
		int best = 2, dist = 0;

		if (q + 65536 > end)
			match_max = end - q;
		else
			match_max = 65536;

#define stb__nc(b,d)  ((d) <= window && ((b) > 9 || stb_not_crap(b,d)))

#define STB__TRY(t,p)  /* avoid retrying a match we already tried */ \
    if (p ? dist != q-t : 1)                             \
    if ((m = stb_matchlen(t, q, match_max)) > best)     \
    if (stb__nc(m,q-(t)))                                \
    best = m, dist = q - (t)

		// rather than search for all matches, only try 4 candidate locations,
		// chosen based on 4 different hash functions of different lengths.
		// this strategy is inspired by LZO; hashing is unrolled here using the
		// 'hc' macro
		h = stb__hc3(q, 0, 1, 2); h1 = STB__SCRAMBLE(h);
		t = chash[h1]; if (t) STB__TRY(t, 0);
		h = stb__hc2(q, h, 3, 4); h2 = STB__SCRAMBLE(h);
		h = stb__hc2(q, h, 5, 6);        t = chash[h2]; if (t) STB__TRY(t, 1);
		h = stb__hc2(q, h, 7, 8); h3 = STB__SCRAMBLE(h);
		h = stb__hc2(q, h, 9, 10);        t = chash[h3]; if (t) STB__TRY(t, 1);
		h = stb__hc2(q, h, 11, 12); h4 = STB__SCRAMBLE(h);
		t = chash[h4]; if (t) STB__TRY(t, 1);

		// because we use a shared hash table, can only update it
		// _after_ we've probed all of them
		chash[h1] = chash[h2] = chash[h3] = chash[h4] = q;

		if (best > 2)
			assert(dist > 0);

		// see if our best match qualifies
		if (best < 3) { // fast path literals
			++q;
		}
		else if (best > 2 && best <= 0x80 && dist <= 0x100) {
			outliterals(lit_start, q - lit_start); lit_start = (q += best);
			stb_out(0x80 + best - 1);
			stb_out(dist - 1);
		}
		else if (best > 5 && best <= 0x100 && dist <= 0x4000) {
			outliterals(lit_start, q - lit_start); lit_start = (q += best);
			stb_out2(0x4000 + dist - 1);
			stb_out(best - 1);
		}
		else if (best > 7 && best <= 0x100 && dist <= 0x80000) {
			outliterals(lit_start, q - lit_start); lit_start = (q += best);
			stb_out3(0x180000 + dist - 1);
			stb_out(best - 1);
		}
		else if (best > 8 && best <= 0x10000 && dist <= 0x80000) {
			outliterals(lit_start, q - lit_start); lit_start = (q += best);
			stb_out3(0x100000 + dist - 1);
			stb_out2(best - 1);
		}
		else if (best > 9 && dist <= 0x1000000) {
			if (best > 65536) best = 65536;
			outliterals(lit_start, q - lit_start); lit_start = (q += best);
			if (best <= 0x100) {
				stb_out(0x06);
				stb_out3(dist - 1);
				stb_out(best - 1);
			}
			else {
				stb_out(0x04);
				stb_out3(dist - 1);
				stb_out2(best - 1);
			}
		}
		else {  // fallback literals if no match was a balanced tradeoff
			++q;
		}
	}

	// if we didn't get all the way, add the rest to literals
	if (q - start < length)
		q = start + length;

	// the literals are everything from lit_start to q
	*pending_literals = (q - lit_start);

	stb__running_adler = stb_adler32(stb__running_adler, start, q - start);
	return q - start;
}

static int stb_compress_inner(stb_uchar *input, stb_uint length)
{
	int literals = 0;
	stb_uint len, i;

	stb_uchar **chash;
	chash = (stb_uchar**)malloc(stb__hashsize * sizeof(stb_uchar*));
	if (chash == NULL) return 0; // failure
	for (i = 0; i < stb__hashsize; ++i)
		chash[i] = NULL;

	// stream signature
	stb_out(0x57); stb_out(0xbc);
	stb_out2(0);

	stb_out4(0);       // 64-bit length requires 32-bit leading 0
	stb_out4(length);
	stb_out4(stb__window);

	stb__running_adler = 1;

	len = stb_compress_chunk(input, input, input + length, length, &literals, chash, stb__hashsize - 1);
	assert(len == length);

	outliterals(input + length - literals, literals);

	free(chash);

	stb_out2(0x05fa); // end opcode

	stb_out4(stb__running_adler);

	return 1; // success
}

stb_uint stb_compress(stb_uchar *out, stb_uchar *input, stb_uint length)
{
	stb__out = out;
	stb__outfile = NULL;

	stb_compress_inner(input, length);

	return stb__out - out;
}

