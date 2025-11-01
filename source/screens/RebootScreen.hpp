#pragma once

#include "Screen.hpp"

class RebootScreen : public Screen {
public:
    RebootScreen();
    ~RebootScreen() override;

    void Draw() override;

    bool Update(Input &input) override;
};
