#pragma once

#include <map>
#include <string>
#include <vector>

#include <filesystem.h>
#include <util/NamedColor.h>

struct Header {
    // inspired by
    // https://codereview.stackexchange.com/questions/38879/parsing-text-file-in-c
    std::string attribute;
    std::string value;

    friend std::istream& operator>>(std::istream& str, Header& data);

    void swap(Header& other);
};

struct Palette {
    fs::path filepath;
    int next;

    std::map<std::string, std::string> header;
    std::vector<NamedColor> colors;

    Palette(fs::path path);

    NamedColor getNext(int inc = 1);
    size_t getIndex();
    void load();
    void reset();

    static void create_default(fs::path path);
};
