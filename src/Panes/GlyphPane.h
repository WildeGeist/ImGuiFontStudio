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

#include <imgui/imgui.h>

#include <stdint.h>
#include <string>
#include <map>

class ProjectFile;
class FontInfos;
class GlyphPane
{
public:
	int DrawGlyphPane(ProjectFile *vProjectFile, int vWidgetId);
	bool LoadGlyph(FontInfos* vFontInfos, ImWchar vCodepoint);

public: // singleton
	static GlyphPane *Instance()
	{
		static GlyphPane *_instance = new GlyphPane();
		return _instance;
	}

protected:
	GlyphPane(); // Prevent construction
	GlyphPane(const GlyphPane&) {}; // Prevent construction by copying
	GlyphPane& operator =(const GlyphPane&) { return *this; }; // Prevent assignment
	~GlyphPane(); // Prevent unwanted destruction};
};

