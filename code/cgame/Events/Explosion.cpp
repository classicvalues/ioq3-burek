#include "cg_local.hpp"
#include "ComplexEventHandler.hpp"

class Explosion final : public EventParser
{
public:
	Explosion( int ev );

	void RegisterAssets() override
	{
		explosionMaterials[0] = trap_R_RegisterShader( "sprites/explo1" );
		explosionMaterials[1] = trap_R_RegisterShader( "sprites/explo2" );
		explosionMaterials[2] = trap_R_RegisterShader( "sprites/explo3" );

		smokeMaterial = trap_R_RegisterShader( "sprites/smoke1" );
		smokeLength = trap_R_AnimationLength( smokeMaterial );

		explosionSounds[0] = trap_S_RegisterSound( "sound/debris/explo1.wav", false );
		explosionSounds[1] = trap_S_RegisterSound( "sound/debris/explo2.wav", false );
		explosionSounds[2] = trap_S_RegisterSound( "sound/debris/explo3.wav", false );
	}

	Vector CRandomVector( Vector max )
	{
		return Vector( crandom() * max.x, crandom() * max.y, crandom() * max.z );
	}

	void Parse( centity_t* cent, Vector position ) override
	{
		EventData ed( cent->currentState );

		localEntity_t* le; // explosion
		localEntity_t* sle; // smoke
		refEntity_t* re;

		float radius = ed.fparm;
		qhandle_t sprite = ed.parm;
		Vector direction = ed.vparm;
		sfxHandle_t sound = ed.sound;

		sprite = cgs.gameMaterials[sprite];
		sound = cgs.gameSounds[sound];

		if ( !ed.parm && !ed.parm2 )
			sprite = explosionMaterials[rand() % 3];

		if ( !ed.sound )
			sound = explosionSounds[rand() % 3];

		if ( ed.parm2 )
		{
			sprite = explosionMaterials[ed.parm2];
			sound = explosionSounds[ed.parm2];
		}

		int spriteAnimationLength = trap_R_AnimationLength( sprite );

		le = CG_MakeExplosion( cent->currentState.origin, direction, 0, sprite, spriteAnimationLength, true );
		trap_S_StartSound( cent->currentState.origin, -1, 0, sound );

		le->light = radius * 5.0f;
		Vector( 1.0f, 0.9f, 0.5f ).CopyToArray( le->lightColor );

		re = &le->refEntity;
		le->radius = radius * 0.5f;
		re->rotation = 0;

		for ( int i = 0; i < 5; i++ )
		{
			Vector r = CRandomVector( Vector( radius * 0.8f, radius * 0.8f, radius * 0.8f ) );
			Vector p = Vector( cent->currentState.origin ) + r;
			Vector ambient, direct, dir;

			trap_R_LightForPoint( p, ambient, direct, dir );
			
			ambient /= 255.0f;
			direct /= 255.0f;

			sle = CG_MakeExplosion( Vector( cent->currentState.origin ) + r, direction, 0, smokeMaterial, smokeLength, true );
			sle->radius = radius * (0.8f + crandom()*0.4f);
			sle->refEntity.rotation = 0;
			sle->color[0] = direct[0];// + ambient[0];
			sle->color[1] = direct[1];// + ambient[1];
			sle->color[2] = direct[2];// + ambient[2];
			sle->color[3] = 3.0f;
		}
	}

private:
	qhandle_t	explosionMaterials[3];
	qhandle_t	smokeMaterial;
	int			smokeLength;
	sfxHandle_t explosionSounds[3];
};

RegisterEventParser( CE_Explosion, Explosion );