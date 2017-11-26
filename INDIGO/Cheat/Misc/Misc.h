#pragma once

#include "../../Engine/Engine.h"


class CMisc
{
public:
//[swap_lines]
	void OnRender();
	void OnCreateMove( CUserCmd* pCmd );
#define POSTPROCESS_VALUE
	void OnDrawModelExecute();
	void OnPlaySound( const char* pszSoundName );

	void OnOverrideView( CViewSetup* pSetup );
	void OnGetViewModelFOV( float& fov );

	vector<int> GetObservervators( int playerId );
	void OnRenderSpectatorList();
//[/swap_lines]
};