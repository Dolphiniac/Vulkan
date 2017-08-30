#pragma once

#include "Quantum/Hephaestus/Declarations.h"

#include <map>


//-----------------------------------------------------------------------------------------------
class HTexture
{
	friend class HMaterial;
	
public:
	static HTexture* CreateOrGetTexture(const class QuString& filepath, ETextureType type = H_TEXTURE_TYPE_2D);
	static HephSampler GetDefaultSampler();

private:
	HTexture(const QuString& filepath, ETextureType type);

private:
	HephWriteDescriptorSet_T GetWriteDescriptorCopy() const;

private:
	HephImage m_image = H_NULL_HANDLE;
	HephImageView m_view = H_NULL_HANDLE;
	HephSampler m_sampler = H_NULL_HANDLE;
	HephWriteDescriptorSet m_writeDescriptorSet = nullptr;
	HephDescriptorImageInfo m_imageDescriptor = nullptr;
	ETextureType m_type;

private:
	static std::map<uint32, HTexture*> s_textures;
	static HephSampler s_defaultSampler;
};