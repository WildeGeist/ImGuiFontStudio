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

#include <memory> // smart ptr
#include <string>

class ProjectFile;
class AbstractPane
{
public:
	int paneWidgetId = 0;
	int NewWidgetId() { return ++paneWidgetId; }

public:
	virtual void Init() = 0;
	virtual void Unit() = 0;
	virtual int DrawPanes(ProjectFile* vProjectFile, int vWidgetId) = 0;
	virtual void DrawDialogsAndPopups(ProjectFile* vProjectFile) = 0;
	virtual int DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas) = 0;
};
