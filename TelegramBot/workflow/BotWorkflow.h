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

    enum class WorkflowMessage {
        STEP_ADD_PHOTO_MESSAGE,
        STEP_SELECT_THEME_MESSAGE,
        STEP_WAITING_FOR_RESULT_MESSAGE,
        STEP_FINISHED_MESSAGE
    };
};