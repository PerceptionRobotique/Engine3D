#include "VRheadset.h"
#include <iostream>

using namespace std;

namespace MIS
{

	VRheadset::VRheadset() : m_mat4EyeRotOffset(glm::rotate(glm::mat4(1.0), -90.0f, glm::vec3(0, 1, 0))), active(false)
	{
	}


	VRheadset::~VRheadset()
	{
		shutdown();
	}


	int VRheadset::initOpenVR()
	{
		vr::EVRInitError eError;
		m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);

		if (!vr::VRSystem() || eError != vr::VRInitError_None)
		{
			vr::VR_Shutdown();
			return -1;
		}


		m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
		m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
		m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
		m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);

		//get the proper resolution of the hmd
		m_pHMD->GetRecommendedRenderTargetSize(&framebufferWidth, &framebufferHeight);

		const std::string& driver = getHMDString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
		const std::string& model = getHMDString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String);
		const std::string& serial = getHMDString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
		const float freq = m_pHMD->GetFloatTrackedDeviceProperty(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
		m_frequency = freq;

		m_mat4HandPos = glm::vec3(0, 0, 0);

		cout << "HMD: " << driver.c_str() << '\t' << model.c_str() << '\t' << serial.c_str() << " ( " << framebufferWidth << " x " << framebufferHeight << " @ " << freq << "Hz )" << endl;

		// Initialize the compositor
		vr::IVRCompositor* compositor = vr::VRCompositor();
		if (!compositor) {
			fprintf(stderr, "OpenVR Compositor initialization failed. See log file for details\n");
			vr::VR_Shutdown();
		}

		// Remove blue circle
		vr::VRSettings()->SetInt32(vr::k_pch_CollisionBounds_Section, vr::k_pch_CollisionBounds_ColorGammaA_Int32, 0);
		//vr::VRSettings()->Sync();

		active = true;

		return 0;
	}

	void VRheadset::shutdown()
	{
		if (m_pHMD != nullptr)
		{
			active = false;
			vr::VR_Shutdown();
			m_pHMD = nullptr;
		}
	}

	std::string VRheadset::getHMDString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError) {
		uint32_t unRequiredBufferLen = m_pHMD->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
		if (unRequiredBufferLen == 0) {
			return "";
		}

		char* pchBuffer = new char[unRequiredBufferLen];
		unRequiredBufferLen = m_pHMD->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
		std::string sResult = pchBuffer;
		delete[] pchBuffer;

		return sResult;
	}


	void VRheadset::getEyeTransformations()
	{
		vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
		const vr::HmdMatrix34_t head = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;

		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 4; ++c) {
				m_mat4eyePosLeft[c][r] = head.m[r][c];
				m_mat4eyePosRight[c][r] = head.m[r][c];
			}
		}

		//	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
		//	{
		//		vr::VRControllerState_t state;
		//		if (m_pHMD->GetTrackedDeviceClass(unDevice) == vr::TrackedDeviceClass_Controller)
		//		{
		//			if (m_pHMD->GetControllerState(unDevice, &state, sizeof(state)))
		//			{
		//				if (getHMDString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String) == "oculus")
		//				{
		//					if (m_pHMD->GetInt32TrackedDeviceProperty(unDevice, vr::Prop_ControllerRoleHint_Int32) == vr::TrackedControllerRole_RightHand)
		//					{
		//						m_mat4EyeRotOffset = glm::rotate(m_mat4EyeRotOffset, -0.05f * state.rAxis[0].x, glm::vec3(0, 1, 0));
		//					}
		//				}
		//				else
		//				{
		//					if (m_pHMD->GetInt32TrackedDeviceProperty(unDevice, vr::Prop_ControllerRoleHint_Int32) == vr::TrackedControllerRole_RightHand && state.ulButtonPressed != 0)
		//					{
		//						m_mat4EyeRotOffset = glm::rotate(m_mat4EyeRotOffset, -0.05f * state.rAxis[0].x, glm::vec3(0, 1, 0));
		//					}
		//				}
		//			}
		//		}
		//	}
		m_mat4eyePosLeft = m_mat4EyeRotOffset * m_mat4eyePosLeft;
		m_mat4eyePosRight = m_mat4EyeRotOffset * m_mat4eyePosRight;
		const vr::HmdMatrix44_t& ltProj = m_pHMD->GetProjectionMatrix(vr::Eye_Left, 0.01f, 1000);
		const vr::HmdMatrix44_t& rtProj = m_pHMD->GetProjectionMatrix(vr::Eye_Right, 0.01f, 1000);

		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				m_mat4ProjectionLeft[c][r] = ltProj.m[r][c];
				m_mat4ProjectionRight[c][r] = rtProj.m[r][c];
			}
		}
		static const glm::vec3 up(0.0, 0.0, 1.0);


		lmatMVP = m_mat4ProjectionLeft * GetHMDMatrixPoseEye(vr::Eye_Left);// *m_mat4eyePosLeft;
		rmatMVP = m_mat4ProjectionRight * GetHMDMatrixPoseEye(vr::Eye_Right);// *m_mat4eyePosRight;
	}

	/*
	void VRheadset::getEyeTransformations()
	{
		vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
		const vr::HmdMatrix34_t head = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;

		glm::mat4 pose;
		std::cout << "POSE NORMAL" << std::endl;
		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 4; ++c) {
				pose[c][r] = head.m[r][c];
				std::cout << pose[c][r]<< "	";

				m_mat4eyePosRight[c][r] = head.m[r][c];
				m_mat4eyePosLeft[c][r] = 0;
			//	m_mat4eyePosRight[c][r] = 0;
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;

		tempoPose.push_back(pose);
		std::cout << "POSE Moyenne" << std::endl;

		for (int i = 0; i <tempoPose.size(); i++)
		{
			for (int r = 0; r < 3; ++r) {
				for (int c = 0; c < 4; ++c) {
					m_mat4eyePosLeft[c][r] += tempoPose[i][c][r];
					//m_mat4eyePosRight[c][r] += tempoPose[i][r][c];
				}
			}
		}

		for (int r = 0; r < 3; ++r) {
			for (int c = 0; c < 4; ++c) {
				m_mat4eyePosLeft[c][r] = m_mat4eyePosLeft[c][r] / tempoPose.size();
				std::cout << m_mat4eyePosLeft[c][r] << "	";

				//m_mat4eyePosRight[c][r] = m_mat4eyePosRight[r][c]/ tempoPose.size();
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;

		if (tempoPose.size() >= 100)
		{
			tempoPose.erase(tempoPose.begin());
		}

		const vr::HmdMatrix44_t& ltProj = m_pHMD->GetProjectionMatrix(vr::Eye_Left, 0.01, 1000);
		const vr::HmdMatrix44_t& rtProj = m_pHMD->GetProjectionMatrix(vr::Eye_Right, 0.01, 1000);

		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				m_mat4ProjectionLeft[c][r] = ltProj.m[r][c];
				m_mat4ProjectionRight[c][r] = rtProj.m[r][c];
			}
		}
		static const glm::vec3 up(0.0, 0.0, 1.0);

		lmatMVP = m_mat4ProjectionLeft * GetHMDMatrixPoseEye(vr::Eye_Left);// *m_mat4eyePosLeft;
		rmatMVP = m_mat4ProjectionRight * GetHMDMatrixPoseEye(vr::Eye_Right);// *m_mat4eyePosRight;
	}*/

	/*
	// Avec filtrage de la pose du casque
	void VRheadset::getEyeTransformations()
	{
		vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
		const vr::HmdMatrix34_t head = trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking;

			for (int r = 0; r < 3; ++r) {
				for (int c = 0; c < 4; ++c) {
					m_mat4eyePosLeft[c][r] = 0.1*head.m[r][c] + (1- 0.1)*tempoPose[c][r];
					m_mat4eyePosRight[c][r] = m_mat4eyePosLeft[c][r];
				}
			}

		const vr::HmdMatrix44_t& ltProj = m_pHMD->GetProjectionMatrix(vr::Eye_Left, 0.01, 1000);
		const vr::HmdMatrix44_t& rtProj = m_pHMD->GetProjectionMatrix(vr::Eye_Right, 0.01, 1000);

		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				m_mat4ProjectionLeft[c][r] = ltProj.m[r][c];
				m_mat4ProjectionRight[c][r] = rtProj.m[r][c];
				tempoPose[c][r] = m_mat4eyePosLeft[c][r];
			}
		}
		static const glm::vec3 up(0.0, 0.0, 1.0);

		lmatMVP = m_mat4ProjectionLeft * GetHMDMatrixPoseEye(vr::Eye_Left);// *m_mat4eyePosLeft;
		rmatMVP = m_mat4ProjectionRight * GetHMDMatrixPoseEye(vr::Eye_Right);// *m_mat4eyePosRight;
	}*/


	void VRheadset::getHandTransformations()
	{
		m_mat4HandPos = glm::vec3(0.0, 0.0, 0.0);
		for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
		{
			vr::VRControllerState_t state;
			if (m_pHMD->GetTrackedDeviceClass(unDevice) == vr::TrackedDeviceClass_Controller)
			{
				if (m_pHMD->GetControllerState(unDevice, &state, sizeof(state)))
				{
					if (getHMDString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String) == "oculus")
					{
						if (m_pHMD->GetInt32TrackedDeviceProperty(unDevice, vr::Prop_ControllerRoleHint_Int32) == vr::TrackedControllerRole_LeftHand)
						{
							m_mat4HandPos.x = 0.1f * state.rAxis[0].y;
							m_mat4HandPos.y = -0.1f * state.rAxis[0].x;
						}
						else if (m_pHMD->GetInt32TrackedDeviceProperty(unDevice, vr::Prop_ControllerRoleHint_Int32) == vr::TrackedControllerRole_RightHand)
						{
							m_mat4HandPos.z = 0.1f * state.rAxis[0].y;
						}
					}
					else
					{
						if (m_pHMD->GetInt32TrackedDeviceProperty(unDevice, vr::Prop_ControllerRoleHint_Int32) == vr::TrackedControllerRole_LeftHand && state.ulButtonPressed != 0)
						{

							m_mat4HandPos.x = 0.5f * state.rAxis[0].y;
							m_mat4HandPos.y = -0.5f * state.rAxis[0].x;
						}
						else if (m_pHMD->GetInt32TrackedDeviceProperty(unDevice, vr::Prop_ControllerRoleHint_Int32) == vr::TrackedControllerRole_RightHand && state.ulButtonPressed != 0)
						{
							m_mat4HandPos.z = 0.5f * state.rAxis[0].y;
						}
						//	axis_value = Vector2(state.rAxis[0].x, state.rAxis[0].y;
					}
				}
			}
		}
	}

	glm::mat4 VRheadset::GetHMDssf(vr::Hmd_Eye nEye)
	{
		vr::HmdMatrix34_t mat = m_pHMD->GetEyeToHeadTransform(nEye);

		return glm::mat4(
			mat.m[0][0], mat.m[1][0], mat.m[2][0], 0,
			mat.m[0][1], mat.m[1][1], mat.m[2][1], 0,
			mat.m[0][2], mat.m[1][2], mat.m[2][2], 0,
			mat.m[0][3], mat.m[1][3], mat.m[2][3], 0
		);
	}


	glm::mat4 VRheadset::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
	{
		vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip);

		return glm::mat4(
			mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
			mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
		);
	}

	glm::mat4 VRheadset::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
	{
		vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(nEye);
		glm::mat4 matrixObj(
			matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
			matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
			matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
			matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

		return glm::inverse(matrixObj);
	}

}