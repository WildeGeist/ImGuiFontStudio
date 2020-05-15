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

#include <string>

class ProjectFile;
class FontInfos;
class FontStructurePane
{
    public:
        int DrawFontStructurePane(ProjectFile *vProjectFile, int vWidgetId);

    private:
        void AnalyzeFont(const std::string& vFilePathName);
        int DisplayAnalyze(int vWidgetId);

    public: // singleton
        static FontStructurePane *Instance()
        {
            static auto *_instance = new FontStructurePane();
            return _instance;
        }

    protected:
        FontStructurePane(); // Prevent construction
        FontStructurePane(const FontStructurePane&) = default; // Prevent construction by copying
        FontStructurePane& operator =(const FontStructurePane&) { return *this; }; // Prevent assignment
        ~FontStructurePane(); // Prevent unwanted destruction
};
