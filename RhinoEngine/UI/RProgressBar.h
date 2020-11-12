//=============================================================================
// RProgressBar.h by Shiyang Ao, 2020 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Core/CoreTypes.h"

// A progress bar UI
class RProgressBar
{
public:
	RProgressBar(int InTotalNum, const std::string& InTitle = "");

	// Start the progress bar with 0 progress
	void Start();

	// Increment the progress by 1
	void Increment();

	// Start a subtask
	void StartSubTask(int InSubTaskTotalNum, const std::string& InSubTaskTitle);

	// End current subtask. Will stop drawing the progress bar for the subtask
	void EndSubTask();

	// Increment the progress of the subtask by 1
	void IncrementSubTask();

protected:
	// Draw the progress bar immediately without waiting for presenting from the render system.
	// This is useful when drawing a progress bar during a slow blocking task.
	void Draw() const;

	int CurrentNum;		// Current number of progress bar
	int TotalNum;		// Max number of progress bar

	std::string Title;

	int CurrentNumSubTask;
	int TotalNumSubTask;
	std::string SubTaskTitle;
};
