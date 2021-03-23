#include "cg_local.hpp"
#include "Events/ComplexEventHandler.hpp"
#include <string>

Client client;

// ===================
// Client::ctor
// ===================
Client::Client()
{
	complexEventHandler = new ComplexEventHandler();
	view = new ClientView();
}

// ===================
// Client::dtor
// ===================
Client::~Client()
{
	delete complexEventHandler;
	delete view;
}

// ===================
// Client::GetCurrentWeapon
// ===================
void Client::Update()
{
	usercmd_t uc = GetUsercmd();

	auto weapon = GetCurrentWeapon();

	if ( weapon )
	{
		if ( uc.interactionButtons & Interaction_PrimaryAttack )
			weapon->OnPrimaryFire();

		if ( uc.interactionButtons & Interaction_SecondaryAttack )
			weapon->OnSecondaryFire();

		if ( uc.interactionButtons & Interaction_TertiaryAttack )
			weapon->OnTertiaryFire();

		if ( uc.interactionButtons & Interaction_Reload )
			weapon->OnReload();
	}
}

// ===================
// Client::PreReload
// ===================
void Client::PreReload()
{

}

// ===================
// Client::PostReload
// ===================
void Client::PostReload()
{

	GetClient()->InitDynamicMusic( cgs.mapname );
	GetClient()->GetEventHandler()->RegisterAssets();
}

// ===================
// Client::GetCurrentWeapon
// ===================
ClientEntities::BaseClientWeapon* Client::GetCurrentWeapon()
{
	if ( cg.snap->ps.weapon < 0 || cg.snap->ps.weapon >= MAX_WEAPONS )
		return nullptr;

	ClientEntities::BaseClientWeapon* weapon = gWeapons[cg.snap->ps.weapon];
	if ( weapon )
		return weapon;

	return nullptr;
}

// ===================
// Client::ParseComplexEvent
// ===================
void Client::ParseComplexEvent( int id, centity_t* cent, Vector position )
{
	if ( !complexEventHandler->ParseComplexEvent( id, cent, position ) )
		CG_Error( "Unknown complex event %i, from ent %i\n", id, cent->currentState.number );
}

// ===================
// Client::RegisterModelConfigData
// ===================
void Client::RegisterModelConfigData( int id, const char* modelName )
{
	if ( nullptr == modelName )
	{
		Util::PrintError( va( "Client::RegisterModelConfigData\nmodelName = nullptr\n" ) );
		return;
	}

	if ( id < 0 || id >= MAX_MODELS )
	{
		Util::PrintError( va( "Client::RegisterModelConfigData:\nout of range (%i) for '%s'\n", id, modelName ) );
		return;
	}

	std::string modelConfig = modelName;
	modelConfig = modelConfig.substr( 0, modelConfig.size() - 4U );
	modelConfig += ".mcfg";

	fileHandle_t f;
	if ( Util::FileOpen( modelConfig.c_str(), &f, FS_READ ) == -1 )
	{
		return; // file doesn't exist, let's silently return
	}

	anims[id] = Assets::ModelConfigData::GetAnimations( modelConfig.c_str() );
}

// ===================
// Client::GetAnimationsForModel
// ===================
std::vector<Assets::ModelAnimation>& Client::GetAnimationsForModel( qhandle_t modelindex )
{
	return anims[modelindex];
}

// ===================
// Client::Time
// ===================
float Client::Time() const
{
	return cg.time * 0.001f;
}

// ===================
// Client::GetUsercmd
// ===================
usercmd_t Client::GetUsercmd() const
{
	usercmd_t uc;
	trap_GetUserCmd( trap_GetCurrentCmdNumber(), &uc );
	return uc;
}

// ===================
// Client::IsPaused
// ===================
bool Client::IsPaused()
{
	return trap_IsPaused();
}

// ===================
// Client::GetView
// ===================
ClientView* Client::GetView()
{
	return view;
}

// ===================
// Client::GetEventHandler
// ===================
ComplexEventHandler* Client::GetEventHandler()
{
	return complexEventHandler;
}

// ===================
// Client::IsLocalClient
// ===================
bool Client::IsLocalClient( centity_t* cent )
{
	if ( nullptr == cent )
		return false;

	if ( cent->currentState.eType != ET_PLAYER )
		return false;

	return cg.clientNum == cent->currentState.clientNum;
}

// ===================
// Client::ExecuteWeaponEvent
// ===================
void Client::ExecuteWeaponEvent( int id, centity_t* cent )
{
	ClientEntities::BaseClientWeapon* weapon = gWeapons[cent->currentState.weapon];
	if ( nullptr == weapon )
		return;

	ClientEntities::BaseClientWeapon::currentPlayer = &cg_entities[cg.clientNum];

	switch ( id )
	{
	case EV_WEAPON_PRIMARY: weapon->OnPrimaryFire(); break;
	case EV_WEAPON_SECONDARY: weapon->OnSecondaryFire(); break;
	case EV_WEAPON_TERTIARY: weapon->OnTertiaryFire(); break;
	case EV_WEAPON_DRAW: weapon->OnDraw(); break;
	case EV_WEAPON_HOLSTER: weapon->OnHolster(); break;
	case EV_WEAPON_RELOAD: weapon->OnReload(); break;
	}
}

// ===================
// Client::InitDynamicMusic
// ===================
void Client::InitDynamicMusic( const char* mapName )
{
	std::string musFile = mapName;

	musFile = musFile.substr( 0, musFile.size() - 4 );
	musFile += ".mus";

	trap_DM_Init( musFile.c_str() );
}

// ===================
// Client::UpdateDynamicMusic
// ===================
void Client::UpdateDynamicMusic( const char* label )
{
	if ( nullptr == label )
	{
		trap_DM_Pause( true, true );
		return;
	}

	trap_DM_Pause( true, false );
	trap_DM_Start( label );
}

// ===================
// GetClient
// ===================
Client* GetClient()
{
	return &client;
}
