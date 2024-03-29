#pragma once

#include "../../Engine/Engine.h"

class CMisc
{
public:
//[swap_lines]
	void OnRender();
	void OnCreateMove( CUserCmd* pCmd );
	void OnDrawModelExecute();
	void OnPlaySound( const char* pszSoundName );

	void OnOverrideView( CViewSetup* pSetup );
	void OnGetViewModelFOV( float& fov );
	void DoAntiAimY(QAngle& angle, int command_number, bool bFlip, bool& clamp);

	vector<int> GetObservervators( int playerId );
	void OnRenderSpectatorList();
//[swap_lines]
};