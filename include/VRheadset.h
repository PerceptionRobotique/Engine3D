#ifndef __VRHEADSET_H__
#define __VRHEADSET_H__

#include <Engine3D_global.h>
#include <openvr/openvr.h>

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

namespace MIS
{

	class ENGINE3D_EXPORT VRheadset
	{
	public:

		VRheadset();
		~VRheadset();
		bool isActive() const;
		int initOpenVR();
		void shutdown();
		glm::vec3 forward = glm::vec3(1, 0, 0);
		glm::vec3 leftright = glm::vec3(0, 1, 0);
		glm::vec3 updown = glm::vec3(0, 0, 1);


		vr::IVRSystem* m_pHMD = nullptr;
		vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];

		void getEyeTransformations();
		void getHandTransformations();
		uint32_t framebufferWidth, framebufferHeight;

		glm::mat4  m_mat4ProjectionLeft;
		glm::mat4  m_mat4ProjectionRight;
		glm::mat4  m_mat4EyeRotOffset;
		glm::mat4  m_mat4eyePosLeft;
		glm::mat4  m_mat4eyePosRight;
		glm::mat4  lmatMVP;
		glm::mat4  rmatMVP;
		glm::mat4  m_matosLeft;
		glm::mat4  m_matosRight;
		//	std::vector<glm::mat4> tempoPose;   // Pour la pile
		glm::mat4 tempoPose;

		glm::vec3  m_mat4HandPos;
		vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
		glm::mat4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
		float m_fNearClip = 0.01f;
		float m_fFarClip = 1000;
		float m_frequency;
		inline std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = NULL)
		{
			uint32_t unRequiredBufferLen = m_pHMD->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);

			char* pchBuffer = new char[unRequiredBufferLen];
			/*unRequiredBufferLen = */m_pHMD->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
			std::string sResult = pchBuffer;
			delete[] pchBuffer;
			return sResult;
		};

		glm::mat4 GetHMDssf(vr::Hmd_Eye nEye);
		static const int noErr = 0;

		uint32_t getWidth() { return framebufferWidth; };
		uint32_t getHeight() { return framebufferHeight; };

	private:
		bool active;

		glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
		glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);

		const int numEyes = 2;


		std::string getHMDString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = nullptr);

	};

}

#endif //__VRHEADSET_H__
