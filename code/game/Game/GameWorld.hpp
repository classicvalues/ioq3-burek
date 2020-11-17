#pragma once

#include <vector>
#include <string>
#include <unordered_map>

class Vector;

class KeyValueLibrary final
{
	using KVMap = std::unordered_map<std::string, std::string>;

public:
	KeyValueLibrary() = default;
	~KeyValueLibrary();

	void			AddKeyValue( const char* key, const char* value );
	std::string&	GetValue( const char* keyName );
	KVMap&			GetMap() { return library; }

	// Templ8s didn't work here for some odd reason so I'm gonna specialise
	const char*		GetCString( const char* keyName, const char* defaultValue );
	float			GetFloat( const char* keyName, float defaultValue );
	int				GetInt( const char* keyName, int defaultValue );
	bool			GetBool( const char* keyName, int defaultValue );
	Vector			GetVector( const char* keyName, Vector defaultValue );

	std::string& operator [] ( const char* val )
	{
		return library[val];
	}

private:
	KVMap library;
};

namespace Entities
{
	class BasePlayer;
}

// ============================
// GameWorld
// Manages whatever is concerned by the
// game's world - player management,
// entity spawning and other facilities
// ============================
class GameWorld final
{
	using KVMap = std::unordered_map<std::string, std::string>;

public:
	constexpr static size_t MaxEntities = MAX_GENTITIES;

public:
	~GameWorld();

	// Deletes entities and stuff
	void			Shutdown();

	// ------ Entity spawning facilities ------

	// Spawns worldspawn, goes on to do some more stuff
	void			SpawnEntities();

	// Spawns worldspawn
	void			SpawnWorldspawn();

	// Dispatches the spawn of an individual entity, from keyvalues
	void			SpawnEntity( KeyValueLibrary& map );

	// Allocates an entity
	template<typename entityType>
	entityType* CreateEntity()
	{
		for ( unsigned int i = MAX_CLIENTS; i < MaxEntities; i++ )
		{
			// Also check for g_entities so we don't get any conflicts
			// on the client & server and whatnot
			if ( gEntities[i] == nullptr && !g_entities[i].inuse )
			{
				gEntities[i] = new entityType();
				gEntities[i]->SetEntityIndex( i );
				return static_cast<entityType*>(gEntities[i]);
			}
		}

		engine->Error( "Exceeded maximum number of entities\n" );
		return nullptr;
	}

	// Allocates an entity at a specified index
	template<typename entityType>
	entityType* CreateEntity( const uint16_t& index )
	{
		if ( nullptr != gEntities[index] )
		{
#ifdef _DEBUG
			engine->Error( va( "Entity slot %d is taken, please consult your programmer\n", index ) );
#else
			engine->Print( va( "Entity slot %d is taken, please consult your programmer\n", index ) );
#endif
			return nullptr;
		}

		gEntities[index] = new entityType();
		gEntities[index]->SetEntityIndex( index );
		gEntities[index]->GetState()->number = index;
		return static_cast<entityType*>(gEntities[index]);
	}

	// ------ Entity interaction ------
	
	// Sweep all entities this entity is touching
	void			TouchTriggers( Entities::IEntity* ent );
	
	// Entity lookup tools

	// Find an entity by its targetname
	Entities::IEntity* FindByName( const char* entityName, Entities::IEntity* lastEntity = nullptr );

	// Find an entity by its classname
	Entities::IEntity* FindByClassname( const char* className, Entities::IEntity* lastEntity = nullptr );

	// Find a random entity by its targetname
	Entities::IEntity* FindByNameRandom( const char* entityName );

	// Find a random entity by its classname
	Entities::IEntity* FindByClassnameRandom( const char* className );

	// ------ Client & players methods ------

	// Locate a client at a spawnpoint
	void			SpawnClient( Entities::BasePlayer* player );

	// Simple spawnpoint finder
	template<typename entityType>
	entityType*		FindSpawnPoint( Vector avoidPoint, bool isBot );

	void			ClientThink( const uint16_t& clientNum );
	void			ClientThink( Entities::BasePlayer* player );

private: // Private client methods, some of which will get moved to a gamemode interface eventually
	void			ClientThinkReal( Entities::BasePlayer* player );
	void			ClientRespawn( Entities::BasePlayer* player );
	void			ClientTimerActions( Entities::BasePlayer* player, int msec );
	void			ClientImpacts( Entities::BasePlayer* player, pmove_t* pm );
	void			ClientEvents( Entities::BasePlayer* player, int oldEventSequence );
	bool			ClientInactivityTimer( Entities::BasePlayer* player );

	void			SendPendingPredictableEvents( Entities::BasePlayer* player );

	void			FindIntermissionPoint();
	void			MoveClientToIntermission( Entities::BasePlayer* player );
	void			ClientIntermissionThink( Entities::BasePlayer* player );
	void			SpectatorThink( Entities::BasePlayer* player );
	void			SpectatorClientEndFrame( Entities::BasePlayer* player );

public:
	void			ClientEndFrame( const uint16_t& clientNum );
	void			ClientEndFrame( Entities::BasePlayer* player );

	// ------ Other ------

	// Reads out keyvalues and populates the stuff
	void			ParseKeyValues();

	// After parsing, the game world will give each 
	// entity its respective list of keyvalue pairs
	void			AssignKeyValuesToEntities();

	// All the entity keyvalue pairs in the map
	// Each KV library represents one entity
	std::vector<KeyValueLibrary> keyValueLibraries;
};

extern GameWorld* gameWorld;
