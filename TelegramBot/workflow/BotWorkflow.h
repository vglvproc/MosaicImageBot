#pragma once

class BotWorkflow {
public:
    enum class WorkflowStep {
        STEP_SELECT_LANGUAGE,
        STEP_ADD_PHOTO,
        STEP_SELECT_THEME,
        STEP_WAITING_FOR_RESULT,
        STEP_FINISHED
    };
};