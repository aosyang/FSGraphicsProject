//=============================================================================
// RProgressBar.cpp by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#include "RProgressBar.h"

#include "imgui/imgui.h"
#include "RenderSystem/RRenderSystem.h"

RProgressBar::RProgressBar(int InTotalNum, const std::string& InTitle /*= ""*/)
	: CurrentNum(0)
	, TotalNum(InTotalNum)
	, Title(InTitle)
{

}

void RProgressBar::Start()
{
	CurrentNum = 0;
	Draw();
}

void RProgressBar::Increment()
{
	CurrentNum = RMath::Min(CurrentNum + 1, TotalNum);
	Draw();
}

void RProgressBar::StartSubTask(int InSubTaskTotalNum, const std::string& InSubTaskTitle)
{
	CurrentNumSubTask = 0;
	TotalNumSubTask = InSubTaskTotalNum;
	SubTaskTitle = InSubTaskTitle;
	Draw();
}

void RProgressBar::EndSubTask()
{
	CurrentNumSubTask = -1;
	TotalNumSubTask = -1;
	SubTaskTitle = "";
}

void RProgressBar::IncrementSubTask()
{
	CurrentNumSubTask = RMath::Min(CurrentNumSubTask + 1, TotalNumSubTask);
	Draw();
}

void RProgressBar::Draw() const
{
	GRenderer.Clear(true, RColor::Black);
	GEngine.BeginImGuiFrame();

	// Draw the loading bar in the center of the screen
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(500, 0));

	const char* str_title = Title.length() > 0 ? Title.c_str() : " ";
	if (ImGui::Begin(str_title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
	{
		float Ratio = RMath::Clamp((float)CurrentNum / (float)TotalNum, 0.0f, 1.0f);
		ImGui::ProgressBar(Ratio, ImVec2(-1, 25));

		// Draw progress bar for a sub-task
		if (TotalNumSubTask > 0)
		{
			const char* str_subtask = SubTaskTitle.length() > 0 ? SubTaskTitle.c_str() : " ";
			ImGui::Text(str_subtask);

			float SubTaskRatio = RMath::Clamp((float)CurrentNumSubTask / (float)TotalNumSubTask, 0.0f, 1.0f);
			ImGui::ProgressBar(SubTaskRatio, ImVec2(-1, 25));
		}

		ImGui::End();
	}

	GEngine.EndImGuiFrame();

	// Present the UI immediately
	GRenderer.Present(false);
}
