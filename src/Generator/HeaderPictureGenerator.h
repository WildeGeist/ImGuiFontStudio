#pragma once

/*
this class will generate a picture representation of the content of a header file
will contain, icon + header name in 2 column i think
to generate with stb. like imgui use stb for rasterize and write an image
*/
#include <Project/FontInfos.h>

#include <imgui/imgui.h>
#include <ctools/cTools.h>
#include <cstdint>
#include <string>
#include <map>

struct GLFWwindow;
class HeaderPictureGenerator
{
public:
	static bool SaveTextureToPng(GLFWwindow* vWin, const std::string& vFilePathName, GLuint vTextureId, ct::uvec2 vTextureSize, uint32_t vChannelCount);
public:
	HeaderPictureGenerator();
	~HeaderPictureGenerator();

	void Generate(const std::string& vFilePathName, FontInfos* vFontInfos);
	void WriteEachGlyphsToPicture(std::map<uint32_t, std::string> vLabels, FontInfos* vFontInfos, uint32_t vHeight);
	void WriteEachGlyphsLabelToPicture(std::map<uint32_t, std::string> vLabels, FontInfos* vFontInfos, uint32_t vHeight);
	void WriteEachGlyphsLabeledToPicture(std::map<uint32_t, std::string> vLabels, FontInfos* vFontInfos, uint32_t vHeight);
};