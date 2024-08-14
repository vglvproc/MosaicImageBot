#pragma once

class BotWorkflow {
public:
    enum class WorkflowMessage {
        STEP_SELECT_LANGUAGE,
        STEP_SELECT_THEME_MESSAGE,
        STEP_SELECT_SIZE,
        STEP_ADD_PHOTO_MESSAGE,
        STEP_WAITING_FOR_RESULT_MESSAGE,
        STEP_FINISHED_MESSAGE,
        CAPTION_ANTI_MOSAIC,
        ERROR_PHOTO_NOT_UPLOADED,
        ERROR_PHOTO_PROCESSING_FAILED
    };
};