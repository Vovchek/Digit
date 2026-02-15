#pragma once

class IImageGeometryProvider {
public:
    virtual ~IImageGeometryProvider() = default;

    virtual bool HasImage() const = 0;
    virtual int GetWidth() const = 0;
    virtual int GetHeight() const = 0;

    // Version increases when image changes
    virtual uint64_t GetImageVersion() const = 0;
};
