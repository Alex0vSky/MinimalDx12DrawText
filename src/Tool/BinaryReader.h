// src\Tool\BinaryReader.h - refactor BinaryReader.h
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Tool {
class BinaryReader {
    uint8_t const* mPos, * mEnd;
    std::unique_ptr<uint8_t[]> mOwnedData;

 public:
	BinaryReader(uint8_t const* dataBlob, size_t dataSize) :
		mPos(dataBlob),
		mEnd(dataBlob + dataSize)
	 {}
    template<typename T> T const& Read() {
        return *ReadArray<T>(1);
    }
    template<typename T> T const* ReadArray(size_t elementCount){
        static_assert(std::is_standard_layout<T>::value, "Can only read plain-old-data types");
        uint8_t const* newPos = mPos + sizeof(T) * elementCount;
        if (newPos < mPos)
            throw std::overflow_error("ReadArray");
        if (newPos > mEnd)
            throw std::runtime_error("End of file");
        auto result = reinterpret_cast<T const*>(mPos);
        mPos = newPos;
        return result;
    }
};
} // namespace prj_3d::MinimalDx12DrawText::Tool
