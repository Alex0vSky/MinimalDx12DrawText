// src\Elem\SpriteQueue.h - refactor ... .h
#pragma once // Copyright 2023 Alex0vSky (https://github.com/Alex0vSky)
namespace prj_3d::MinimalDx12DrawText::Elem { class SpriteQueue {
    static constexpr size_t InitialQueueSize = 64;
	template<typename TDerived> 
	struct AlignedNew {
		// Allocate aligned memory.
		static void* operator new (size_t size) {
			const size_t alignment = alignof(TDerived);
			static_assert(alignment > 8, "AlignedNew is only useful for types with > 8 byte alignment. Did you forget a __declspec(align) on TDerived?");
			static_assert(((alignment - 1) & alignment) == 0, "AlignedNew only works with power of two alignment");
#			ifdef _WIN32
			void* ptr = _aligned_malloc(size, alignment);
#			else
			// This C++17 Standard Library function is currently NOT
			// implemented for the Microsoft Standard C++ Library.
			void* ptr = aligned_alloc(alignment, size);
#			endif
			if ( !ptr )
				throw std::bad_alloc( );
			return ptr;
		}
		// Free aligned memory.
		static void operator delete (void* ptr) {
#			ifdef _WIN32
			_aligned_free( ptr );
#			else
			free( ptr );
#			endif
		}
		// Array overloads.
		static void* operator new[](size_t size) {
			static_assert((sizeof(TDerived) % alignof(TDerived) == 0), "AlignedNew expects type to be padded to the alignment");
			return operator new(size);
		}
		static void operator delete[](void* ptr) {
			operator delete(ptr);
		}
	};
    size_t m_spriteQueueCount = 0;
    size_t m_spriteQueueArraySize = 0;

 public:
	// Info about a single sprite that is waiting to be drawn.
#		if (DIRECTX_MATH_VERSION < 315)
			__declspec( align( 16 ) ) struct
#		else
			 XM_ALIGNED_STRUCT( 16 ) 
#		endif
		Info : public AlignedNew<Info> {
		XMFLOAT4A source, destination, color, originRotationDepth;
		D3D12_GPU_DESCRIPTOR_HANDLE texture;
		XMVECTOR textureSize;
		unsigned int flags;
		// Combine values from the public SpriteEffects enum with these internal-only flags.
		static constexpr unsigned int SourceInTexels = 4;
		static constexpr unsigned int DestSizeInPixels = 8;
		enum SpriteEffects : uint32_t
		{
			SpriteEffects_None = 0,
			SpriteEffects_FlipHorizontally = 1,
			SpriteEffects_FlipVertically = 2,
			SpriteEffects_FlipBoth = SpriteEffects_FlipHorizontally | SpriteEffects_FlipVertically,
		};
		static_assert((SpriteEffects_FlipBoth & (SourceInTexels | DestSizeInPixels)) == 0, "Flag bits must not overlap");
	};

	void grow() {
		if ( m_spriteQueueCount < m_spriteQueueArraySize ) 
			return;
		// Grow by a factor of 2.
		const size_t newSize = std::max( InitialQueueSize, m_spriteQueueArraySize * 2 );
		// Allocate the new array.
		auto newArray = std::make_unique<Info[]>( newSize );
		// Copy over any existing sprites.
		for ( size_t i = 0; i < m_spriteQueueCount; i++ )
			newArray[ i ] = m_spriteQueue[ i ];
		// Replace the previous array with the new one.
		m_spriteQueue = std::move( newArray );
		m_spriteQueueArraySize = newSize;
	}
	Info* current() const {
		return &m_spriteQueue[ m_spriteQueueCount ];
	}
	void next() {
		m_spriteQueueCount++;
	}
	std::vector< Info const* > toPlain() {
	    std::vector< Info const* > plainSprites;
		plainSprites.resize( m_spriteQueueCount );
		for ( size_t i = 0; i < m_spriteQueueCount; i++ )
			plainSprites[ i ] = &m_spriteQueue[ i ];
		return plainSprites;
	}
	inline size_t count() const {
		return m_spriteQueueCount;
	}
	inline D3D12_GPU_DESCRIPTOR_HANDLE textureHandleHead() const {
		return m_spriteQueue[ 0 ].texture;
	}

 private:
    // Queue of sprites waiting to be drawn.
    std::unique_ptr< Info[] > m_spriteQueue;
};} // namespace prj_3d::MinimalDx12DrawText::Elem