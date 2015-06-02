/*******************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2011-2015 Intel Corporation. All Rights Reserved.

*******************************************************************************/
#pragma once

#include "xwalk/sysapps/realsense/sp_math_utils.h"

#include "PXCSenseManager.h"
#include "PXCScenePerception.h"
#include "pxcprojection.h"
#include "pxcmetadata.h"
#include <iostream>
#include <windows.h>

class IScenePerceptionMeshFetcher
{
public:
	IScenePerceptionMeshFetcher() {}
	virtual pxcStatus DoMeshingUpdate(PXCBlockMeshingData *blockMeshingData, PXCScenePerception::MeshingUpdateInfo  &meshingUpdateInfo) = 0;
	virtual PXCBlockMeshingData* CreatePXCBlockMeshingData(const pxcI32 maxBlockMesh, const pxcI32 maxVertices, const pxcI32 maxFaces, const pxcBool bUseColor) = 0;
	virtual ~IScenePerceptionMeshFetcher() {};
};

class ScenePerceptionController : public IScenePerceptionMeshFetcher
{
	public:
		enum COORDINATE_SYSTEM
		{
			RSSDK_DEFAULT,
			RSSDK_OPENCV
		};
		~ScenePerceptionController()
		{
			if(m_pProjection)
			{
				m_pProjection->Release();
			}

			if(m_pSenseMgr)
			{
				m_pSenseMgr->Close();
				m_pSenseMgr->Release();
			}

			if(m_pSession)
				m_pSession->Release();
		}

		pxcStatus InitializeProjection()
		{
			if(!m_pSenseMgr)
			{
				return PXC_STATUS_ALLOC_FAILED;
			}
			PXCCapture::Device *pDevice = m_pSenseMgr->QueryCaptureManager()->QueryDevice();
			if(!pDevice)
			{
				return PXC_STATUS_ITEM_UNAVAILABLE;
			}
			m_pProjection = pDevice->CreateProjection();
			return (m_pProjection != NULL) ? PXC_STATUS_NO_ERROR: PXC_STATUS_ALLOC_FAILED;
		}

		PXCProjection* QueryProjection()
		{
			return m_pProjection;
		}

		ScenePerceptionController(const int   uiColorWidth,
															const int   uiColorHeight,
															const float fColorFPS,
															const int   uiDepthWidth,
															const int   uiDepthHeight,
															const float fDepthFPS) : m_pSession(NULL),
															  m_pScenePerception(NULL),
															  m_pSenseMgr(NULL),
															  m_pProjection(NULL),
															  m_pPreviewImage(NULL),
															  m_coordinateSystem(RSSDK_OPENCV),
															  m_bIsPipelineInitialized(false),
															  m_bIsPlaybackOrRecording(false)
		{
			m_pSession = PXCSession::CreateInstance();
			if(!m_pSession)
			{
				std::cout << "Failed to Create Session" << std::endl;
				return;
			}

			m_pSenseMgr = m_pSession->CreateSenseManager();
			m_pSession->SetCoordinateSystem(PXCSession::COORDINATE_SYSTEM_REAR_OPENCV);
			if(!m_pSenseMgr)
			{
				std::cout << "Failed to create an SDK SenseManager" << std::endl;
				return;
			}

			m_pSenseMgr->EnableStream(PXCCapture::STREAM_TYPE_COLOR, uiColorWidth, uiColorHeight, fColorFPS);
			m_pSenseMgr->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, uiDepthWidth, uiDepthHeight, fDepthFPS);
			m_coordinateSystem = (!(m_pSession->QueryCoordinateSystem() & PXCSession::COORDINATE_SYSTEM_REAR_OPENCV)) ? RSSDK_DEFAULT: RSSDK_OPENCV;

			pxcStatus sts = m_pSenseMgr->EnableScenePerception();
			if(sts < PXC_STATUS_NO_ERROR)
			{
				std::cout << "Failed to enable scene perception module" << std::endl;		
				return;
			}

			m_pScenePerception = m_pSenseMgr->QueryScenePerception();
			if(m_pScenePerception == NULL)
			{
				std::cout << "Failed to Query scene perception module" << std::endl;		
				return;
			}
		}
		operator bool()
		{
			return (m_pScenePerception) ? true : false;
		}
		bool QueryCaptureSize(int &uiColorWidth, int &uiColorHeight, int &uiDepthWidth, int &uiDepthHeight)
		{
			if(m_bIsPipelineInitialized)
			{
				PXCSizeI32 streamSize = { 0 };
				PXCCaptureManager *captureMgr = m_pSenseMgr->QueryCaptureManager();
				streamSize = captureMgr->QueryImageSize(PXCCapture::STREAM_TYPE_COLOR),
				uiColorWidth = streamSize.width;
				uiColorHeight = streamSize.height;

				streamSize = captureMgr->QueryImageSize(PXCCapture::STREAM_TYPE_DEPTH),
				uiDepthWidth  = streamSize.width;
				uiDepthHeight =  streamSize.height;
				return true;
			}
			return false;
		}

		bool IsPlaybackOrRecording() const
		{
			return m_bIsPlaybackOrRecording;
		}

		bool SetVoxelResolution(PXCScenePerception::VoxelResolution voxelResolution)
		{
			if(!m_bIsPipelineInitialized && m_pSession && m_pSenseMgr && m_pScenePerception)
			{
				if(m_pScenePerception->SetVoxelResolution(voxelResolution) == PXC_STATUS_NO_ERROR)
					return true;
			}
			return false;
		}
		
		bool InitPipeline(PXCSenseManager::Handler* handler)
		{
			if(!m_pSession || !m_pSenseMgr || !m_pScenePerception)
			{
				return false;
			}
			pxcStatus sts = m_pSenseMgr->Init(handler); 
			if(sts < PXC_STATUS_NO_ERROR) 
			{
				std::cout << "SenseManager Init Failed\n";
				return false;
			}
			m_bIsPipelineInitialized = true;
			return true;
		}
		
		PXCBlockMeshingData* CreatePXCBlockMeshingData(const pxcI32 maxBlockMesh, const pxcI32 maxVertices, const pxcI32 maxFaces, const pxcBool bUseColor)
		{
			PXCBlockMeshingData* pBlockMeshingData = NULL;
			if(m_pScenePerception)
			{			
				pBlockMeshingData  = m_pScenePerception->CreatePXCBlockMeshingData(maxBlockMesh, maxVertices, maxFaces, bUseColor);
			}
			return pBlockMeshingData;
		}


		float GetDimension()
		{
			float fDimension = 0.0f;
			if(m_pScenePerception)
			{
				switch(m_pScenePerception->QueryVoxelResolution())
				{
					case PXCScenePerception::LOW_RESOLUTION:
						fDimension = 4.0f;
						break;
					case PXCScenePerception::MED_RESOLUTION:
						fDimension = 2.0f;
						break;
					case PXCScenePerception::HIGH_RESOLUTION:
						fDimension = 1.0f;
						break;
				}
			}
			return fDimension;
		}

		void ScenePerceptionController::PauseScenePerception(bool bPause)
		{
			m_pSenseMgr->PauseScenePerception(bPause);
		}
		
		COORDINATE_SYSTEM ScenePerceptionController::GetCoordinateSystem()
		{
			return m_coordinateSystem;
		}

		bool GetCameraParameters(float &fx, float &fy, float &u0, float &v0)
		{
			if(!m_pSenseMgr)
				return false;
			PXCCapture::Device *pDevice = m_pSenseMgr->QueryCaptureManager()->QueryDevice();
			if(!pDevice)
			{
				std::cout << "No Device associated with Session" << std::endl;
				return false;
			}
			PXCPointF32 pxcF32ColorFocalLength = pDevice->QueryColorFocalLength();
			fx = pxcF32ColorFocalLength.x;
			fy = pxcF32ColorFocalLength.y;

			PXCPointF32 pxcF32ColorPrincipalPoint = pDevice->QueryColorPrincipalPoint();
			u0 = pxcF32ColorPrincipalPoint.x;
			v0 = pxcF32ColorPrincipalPoint.y;	

			return true;
		}

		void EnableReconstruction(bool bEnable)
		{
			m_pScenePerception->EnableSceneReconstruction(bEnable);
		}
		
		bool ProcessNextFrame(PXCImage **color, PXCImage **depth, pxcF32 *curPose, PXCScenePerception::TrackingAccuracy& accuracy, float& imageQuality)
		{	
			m_pPreviewImage = NULL;
			auto pxcStatus = m_pSenseMgr->AcquireFrame(true);
			if(pxcStatus < PXC_STATUS_NO_ERROR)
			{
				if(pxcStatus == PXC_STATUS_DEVICE_LOST)
				{		
					std::cout << "Capture Device Lost" << std::endl;
				}
				return false;
			}

			PXCCapture::Sample *sample = m_pSenseMgr->QueryScenePerceptionSample();

			if (!sample)
			{
				// If the SP module is paused, the sample will be NULL.  Query the raw color/depth images
				// to support live preview and calculation of scene quality
				sample = m_pSenseMgr->QuerySample();

				if (!sample)
					return false;
				if(!sample->color || !sample->depth)
				{
					return false;
				}
				if (color) *color = sample->color;
				if (depth) *depth = sample->depth;
				imageQuality = m_pScenePerception->CheckSceneQuality(sample);
				return true;
			}

			if (color) *color = sample->color;
			if (depth) *depth = sample->depth;

			if(curPose) 
			{
				m_pScenePerception->GetCameraPose(curPose);
				accuracy = m_pScenePerception->QueryTrackingAccuracy();
			}
			return true;
		}

		PXCImage* QueryVolumePreview(const AppUtils::PoseMatrix4f& renderMatrix)
		{
			return (m_pPreviewImage = m_pScenePerception->QueryVolumePreview(renderMatrix.m_data));
		}

		// Release any data after the frame has been processed in the UI thread
		void CleanupFrame()
		{
			if (m_pPreviewImage) 
			{
				m_pPreviewImage->Release(); 
				m_pPreviewImage = NULL; 
			}
			m_pSenseMgr->ReleaseFrame();
		}

		// Called by the meshing thread to collect new meshing data.  It does not need to be synchronized to ProcessNextFrame
		pxcStatus DoMeshingUpdate(PXCBlockMeshingData *blockMeshingData, PXCScenePerception::MeshingUpdateInfo  &meshingUpdateInfo/*pxcBool bUpdateColors*/)
		{
			pxcStatus status = m_pScenePerception->DoMeshingUpdate(blockMeshingData, 0, &meshingUpdateInfo);		
			m_pScenePerception->SetMeshingThresholds(0.03f, 0.005f);		
			return status;
		}
		// Reset the scene perception module state without destroying the entire pipeline and re-creating it.
		// Also set the initial pose of the camera
		bool ResetScenePerception()
		{
			m_pScenePerception->SetMeshingThresholds(0.0f, 0.0f);
			const auto pxcSts = m_pScenePerception->Reset();
			return (pxcSts >= PXC_STATUS_NO_ERROR) ? true: false;
		}

		PXCScenePerception * QueryScenePerception()
		{
			return m_pScenePerception;
		}
		
		bool StreamFrames(bool blocking)
		{
			if (m_pSenseMgr->StreamFrames(blocking) < PXC_STATUS_NO_ERROR) {
				return false;
			}
			return true;
		}
		
	private:
		PXCSession *m_pSession;
		PXCScenePerception* m_pScenePerception;
		PXCSenseManager* m_pSenseMgr;
		PXCProjection * m_pProjection;
		PXCImage *m_pPreviewImage;
		COORDINATE_SYSTEM m_coordinateSystem;
		ScenePerceptionController();
		ScenePerceptionController(ScenePerceptionController &);
		ScenePerceptionController& operator=(ScenePerceptionController &);
		bool			m_bIsPipelineInitialized;
		bool			m_bIsPlaybackOrRecording;
};