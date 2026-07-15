#pragma once

#include "config.hpp"

[[nodiscard]] bool runCommand(const Command &command, const QProcessEnvironment &environment);
void focusKakaoTalk(const Config &config);
void terminateKakaoTalk(const Config &config);
