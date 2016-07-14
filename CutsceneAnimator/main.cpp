#include <SFML/Graphics.hpp>
#include <iostream>
#include <list>
#include <string>
#include "Tileset.h"
#include <sstream>
#include <assert.h>
#include <map>
#include "GUI.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include <set>

using namespace std;
using namespace sf;

#define PI 3.14159

struct Entity;
//const int NUM_FRAMES = 60;

list<Tileset*> tilesetList;
list<Entity*> selectedEntities;
list<Entity*> allEntities;

string fName;
int currentFrame;
int currentLayer;
//int totalLayers;
//int totalFrames;
bool drawingRect;
Vector2f pressPos;
bool mousePressed;
bool entityMove;
bool entityRotate;
bool entityScaleX;
bool entityScaleY;
bool entityScaleXY;
int selectedPointIndex;
Vector2f pastPos;
list<Entity*> copiedEntities;
int copyFrame;
Panel *showPanel;
Panel *entitySelectPanel;
bool creatingEntity;
Vector2f createPos;
sf::CircleShape tempCreateMarker;
string createTypeName;

//Panel *birdPanel;

float rotatePressAngle;
sf::CircleShape transformCircles[8];
Vector2f transformPoints[8];

int transformRotationRadius;
int transformScaleRadius;

//list<CamInfo> camera;

sf::RenderWindow *window;


bool panning;
Vector2f panStart;


struct StateInfo
{
	string name;
	int frame;
};

struct EntityType
{
	list<Tileset*> tSets;
	Tileset *ts_rep;
	Panel *panel;
	GridSelector *gs;
	int **frame;
	string name;
};

map<string,EntityType*> entityTypes;
list<EntityType*> entityTypeList;

StateInfo birdInfo[7][7];


struct Entity;
Entity * CreateEntity( /*const std::string &typeName*/ );
void SetEntity( Entity *ent, int frame, Tileset *ts, int tsIndex, bool facingRight, 
	Vector2f &pos, float angle, Vector2f &scale );
struct GUI : GUIHandler
{
	void ButtonCallback( Button *b, const std::string & e )
	{
		
	}

	void TextBoxCallback( TextBox *tb, const std::string & e )
	{
	}
	
	void GridSelectorCallback( GridSelector *gs, const std::string & p_name )
	{
		string name = p_name;
		Panel *panel = gs->owner;
		if( panel == entitySelectPanel )
		{
			if( name != "not set" )
			{
				//cout << "ENTITY SELECT PANEL SELECTEd" << endl;
				showPanel = entityTypes[name]->panel;
				createTypeName = name;
				//createPos = ;
				//create new entity here
				//entityTypes[name]->
			}
		}
		else
		//if( panel = entityTypes[name]->panel )
		{
			if( name != "not set" )
			{
				int selX = gs->selectedX;
				int selY = gs->selectedY;

				if( creatingEntity )
				{
					EntityType *et = entityTypes[createTypeName];
					//createPos
					Tileset *ts = NULL;
					for( list<Tileset*>::iterator it = et->tSets.begin(); it != et->tSets.end(); ++it )
					{
						cout << (*it)->sourceName << "\t-----" << endl;
						if( (*it)->sourceName == name )
						{
							ts = (*it);
						}
					}
					assert( ts != NULL );
					Entity *e = CreateEntity( );//name );
					SetEntity( e, currentFrame, ts, et->frame[selX][selY],
						true, createPos, 0, Vector2f( 1, 1 ) );
					showPanel = NULL;
					creatingEntity = false;
				}
				else
				{
					//just editing
				}

				cout << "name: " << name << endl;
				cout << "frame: " << entityTypes[name]->frame[selX][selY];
				/*showPanel = NULL;
				if( birdInfo[selX][selY].name == "glide" )
				{

				}
				else if( birdInfo[selX][selY].name == "wing" )
				{
				}
				else if( birdInfo[selX][selY].name == "kick" )
				{
				}
				else if( birdInfo[selX][selY].name == "intro" )
				{
				}*/
				//cout << "selx: " << selX << ", " << selY << endl;
			}
			else
			{
				cout << "not set" << endl;
			}
		}
		//cout << "blah" << endl;
	}

	void CheckBoxCallback( CheckBox *cb, const std::string & e )
	{
	}
};

struct CamInfo
{
	CamInfo()
	{
		view.setCenter( 0, 0 );
		view.setSize( 1920, 1080 );
		zoomLevel = 40;
		angleLevel = 0;
	}
	View view;
	int zoomLevel;
	int angleLevel;
};

//Entity *LoadEntity( 



//map<int, CamInfo> camera;
list<CamInfo> camera;

CamInfo & GetCamInfo( int frame )
{
	//cout << "frame: " << frame << endl;
	int index = 1;
	for( list<CamInfo>::iterator it = camera.begin(); it != camera.end(); ++it )
	{
		if( index == frame )
		{
			return (*it);
		}
		++index;
	}

	assert( false );
}

list<CamInfo>::iterator GetCamIter( int frame )
{
	int index = 1;
	for( list<CamInfo>::iterator it = camera.begin(); it != camera.end(); ++it )
	{
		if( index == frame )
		{
			return it;
		}
		++index;
	}

	assert( false );
}

float length( Vector2f &v )
{
	return sqrt( v.x * v.x + v.y * v.y );
}

sf::Vector2f normalize( Vector2f v )
{
	float vLen = length( v );
	if( vLen > 0 )
		return sf::Vector2<float>( v.x / vLen, v.y / vLen );
	else
		return sf::Vector2<float>( 0, 0 );
}

float dot( sf::Vector2f a, sf::Vector2f b )
{
	float ax = a.x;
	float ay = a.y;
	float bx = b.x;
	float by = b.y;
	return ax * bx + ay * by;
}

double cross( sf::Vector2f a, sf::Vector2f b )
{
	float ax = a.x;
	float ay = a.y;
	float bx = b.x;
	float by = b.y;
	return ax * by - ay * bx;
	//return a.x * b.y - a.y * b.x;
}

bool QuadContainsPoint( Vector2f &A, Vector2f &B, Vector2f&C, Vector2f&D, Vector2f &point )
{
	Vector2f AB = B - A;
	Vector2f AD = D - A;
	Vector2f pointA = point - A;
	float pointAB = dot( pointA, normalize( AB ) );
	float pointAD = dot( pointA, normalize( AD ) );

	if( pointAB >= 0 && pointAB * pointAB <= dot( AB,AB ) )
	{
		if( pointAD >= 0 && pointAD * pointAD <= dot( AD, AD ) )
		{
			return true;
		}
	}

	return false;
}

Tileset * GetTileset( const string & s, int tileWidth, int tileHeight )
{
	for( list<Tileset*>::iterator it = tilesetList.begin(); it != tilesetList.end(); ++it )
	{
		if( (*it)->sourceName == s )
		{
			return (*it);
		}
	}


	//not found


	Tileset *t = new Tileset();
	t->texture = new Texture();
	t->texture->loadFromFile( s );
	t->tileWidth = tileWidth;
	t->tileHeight = tileHeight;
	t->sourceName = s;
	tilesetList.push_back( t );

	return t;
	//make sure to set up tileset here
}

struct SprInfo
{
	SprInfo()
	{
		facingRight = true;
		selected = false;
		ts = NULL;
	}
	Sprite sprite;
	bool facingRight;
	bool selected;
	Tileset *ts;
	int frame;
};

struct Entity
{
	void SetFaceRight( int frame, bool right );
	void Draw( int frame, sf::RenderTarget *target );

	list<SprInfo*> images;
	SprInfo * GetSprInfo( int frame );
	list<SprInfo*>::iterator GetSprIter( int frame );
	void FlipX( int frame );
	void FlipY( int frame );
	//map<int,SprInfo> images;
	Vector2f GetPoint( int index );
	int layer;
};

void NewBlankFrame()
{
	bool back = currentFrame == camera.size();
	for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
	{
		

		if( back )
		{
			(*it)->images.push_back( (SprInfo*)NULL );
		}
		else
		{
			list<SprInfo*>::iterator sprit = (*it)->GetSprIter( currentFrame + 1 );
			list<SprInfo*> &sprList = (*it)->images;
			sprList.insert( sprit, (SprInfo*)NULL );
		}
		
		
		//(*sprit)
	}

	CamInfo c;
	if( back )
	{
		camera.push_back( c );
	}
	else
	{
		list<CamInfo>::iterator cit = GetCamIter( currentFrame + 1 );
		camera.insert( cit, c );
	}
	
	++currentFrame;
}

void NewCopyFrame()
{
	bool back = currentFrame == camera.size();
	for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
	{
		list<SprInfo*>::iterator sprit = (*it)->GetSprIter( currentFrame );

		SprInfo *nSpr = new SprInfo( *(*sprit) );
		if( back )
		{
			(*it)->images.push_back( nSpr );
		}
		else
		{
			sprit = (*it)->GetSprIter( currentFrame + 1 );
			list<SprInfo*> &sprList = (*it)->images;

			sprList.insert( sprit, nSpr );
			//sprList.insert( sprit, (SprInfo*)NULL );
		}
		
		
		//(*sprit)
	}

	CamInfo &c = GetCamInfo( currentFrame );
	if( back )
	{
		camera.push_back( c );
	}
	else
	{
		list<CamInfo>::iterator cit = GetCamIter( currentFrame + 1 );
		camera.insert( cit, c );
	}
	
	//++currentFrame;
	//for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
	//{
	//	list<SprInfo*>::iterator sprit = (*it)->GetSprIter( currentFrame );
	//	list<SprInfo*> &sprList = (*it)->images;

	//	
	//	//(*sprit)
	//}

	++currentFrame;
}

void CopyEntityFrame()
{
	bool back = (currentFrame == camera.size());
	if( selectedEntities.size() == 1 )
	{
		Entity *ent = selectedEntities.front();
		
		list<SprInfo*>::iterator sprit = ent->GetSprIter( currentFrame );

		SprInfo *nSpr = new SprInfo( *(*sprit) );
		if( back )
		{
			NewBlankFrame();
			camera.back() = GetCamInfo( currentFrame - 1 );
			ent->images.back() = nSpr;
		}
		else
		{
			sprit = ent->GetSprIter( currentFrame + 1 );
			list<SprInfo*> &sprList = ent->images;

			sprList.insert( sprit, nSpr );
			//sprList.insert( sprit, (SprInfo*)NULL );
		}
	}
}

void RemoveCurrentFrame()
{
	
	/*camera.pop_front();
	for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
	{
		(*it)->images.pop_front();
	}
	cout << "currFrame: " << currentFrame << endl;
	return;*/

	int numFrames = camera.size();

	if( numFrames == 1 )
	{
		return;
	}

	list<CamInfo>::iterator cit = GetCamIter( currentFrame );
	if( currentFrame == 1 )
	{
		//cout << "camera pop front" << endl;
		camera.pop_front();
	}
	else if( currentFrame == numFrames )
	{
		camera.pop_back();
	}
	else
	{
		camera.erase( cit );
	}

	//cout << "camera size is now: " << camera.size() << endl;

	//cout << "removing frame after camera stuff" << endl;
	for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
	{
		list<SprInfo*>::iterator sprinfoIt = (*it)->GetSprIter( currentFrame );

		if( (*sprinfoIt) != NULL )
		{
			delete (*sprinfoIt);
			//cout << "deleting sprite info" << endl;
		}

		if( currentFrame == numFrames )
		{
			(*it)->images.pop_back();
			//cout << "popping back" << endl;
			--currentFrame;
		}
		else if( currentFrame == 1 )
		{
			(*it)->images.pop_front();
			//cout << "popping front! size: " << (*it)->images.size() << endl;
			
		}
		else
		{
			(*it)->images.erase( sprinfoIt );
			//cout << "erasing" << endl;
		}
		
		//cout << "erasing from " << endl;
		//currentFrame--;
	}
	//cout << "rfend" << endl;
	


	//cout << "num frame: " << camera.size() << endl;
	//for( list<CamInfo>::iterator it = camera.begin(); it != camera.end(); ++it )
	//{
//	}
}

SprInfo * Entity::GetSprInfo( int frame )
{
	
	int index = 1;
	for( list<SprInfo*>::iterator it = images.begin(); it != images.end(); ++it )
	{
		if( index == frame )
		{
			return (*it);
		}
		//if( (*it)->
		++index;
	}

	cout << "frame: " << frame << ", image size: " << images.size() << endl;
	assert( false );
	return NULL;
}

list<SprInfo*>::iterator Entity::GetSprIter( int frame )
{
	int index = 1;
	for( list<SprInfo*>::iterator it = images.begin(); it != images.end(); ++it )
	{
		if( index == frame )
		{
			return it;
		}
		//if( (*it)->
		++index;
	}

	assert( false );
}

//Vector2f Entity::GetPoint( int frame, int index )
void UpdateTransformPoints()
{
	assert( selectedEntities.size() == 1 );
	Entity *e = selectedEntities.front();
	
	SprInfo *sprInfo = e->GetSprInfo(currentFrame);
	if( sprInfo == NULL )
		return;

	Sprite &spr = sprInfo->sprite;
	Transform t;
	t.rotate( spr.getRotation() );
	t.scale( spr.getScale() );
	Vector2f halfSize( spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2 );
	//Vector2f topLeft = spr.getPosition() - halfSize;

	for( int i = 0; i < 9; ++i )
	{
		int index = i;
		if( i == 4 )
		{
			continue;
		}
		else if( i > 4 )
		{
			index--;
		}

		int x = i % 3;
		int y = i / 3;

		Vector2f off;
		if( x == 0 )
		{
			off.x = -halfSize.x;
		}
		else if( x == 1 )
		{
			off.x = 0;
		}
		else if( x == 2 )
		{
			off.x = halfSize.x;
		}

		if( y == 0 )
		{
			off.y = -halfSize.y;
		}
		else if( y == 1 )
		{
			off.y = 0;
		}
		else if( y == 2 )
		{
			off.y = halfSize.y;
		}

		
		
		off = t.transformPoint( off );
		transformPoints[index] = spr.getPosition() + off;
	}
}

void Entity::Draw( int frame, RenderTarget *target )
{
	SprInfo *sprInfo = GetSprInfo(frame);
	if( sprInfo != NULL )
		target->draw( sprInfo->sprite );
}

void Entity::SetFaceRight( int frame, bool right )
{
	SprInfo &oldInfo = *GetSprInfo(frame);
	//bool &old = images[frame].facingRight;
	if( oldInfo.facingRight != right )
	{
		sf::IntRect r = oldInfo.sprite.getTextureRect();
		oldInfo.sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
		oldInfo.facingRight = right;
		//facingRight = right;
	}
}

void Entity::FlipX( int frame )
{
	SprInfo &info = *GetSprInfo(frame);
	
	IntRect ir = info.sprite.getTextureRect();
	ir.left += ir.width;
	ir.width = -ir.width;
	info.sprite.setTextureRect( ir );
}

void Entity::FlipY( int frame )
{
	SprInfo &info = *GetSprInfo(frame);
	IntRect ir = info.sprite.getTextureRect();
	ir.top += ir.height;
	ir.height = -ir.height;
	info.sprite.setTextureRect( ir );
}

void OpenFromFile( const string &fullPath )
{
	std::ifstream is;
	is.open( fullPath );

	if( !is.is_open() )
	{
		assert(  "file was invalid" && 0 );
	}

	int numUniqueTilesets;
	is >> numUniqueTilesets;

	Tileset **tilesets = new Tileset*[numUniqueTilesets];
	for( int i = 0; i < numUniqueTilesets; ++i )
	{
		string sourceName;
		is >> sourceName;
		int tileX, tileY;
		is >> tileX;
		is >> tileY;

		tilesets[i] = GetTileset( sourceName, tileX, tileY );
		
		//is >> 

		//tilesets[i] = GetTil
		
	}

	int numTotalFrames;
	is >> numTotalFrames;

	for( int i = 0; i < numTotalFrames; ++i )
	{
		int centerX, centerY, zoomLevel, angleLevel;
		is >> centerX >> centerY >> zoomLevel >> angleLevel;

		camera.push_back( CamInfo() );
		CamInfo &ci = camera.back();
		ci.view.setCenter( Vector2f( centerX, centerY ) );
		ci.zoomLevel = zoomLevel;
		ci.angleLevel = angleLevel;
		//ci.view.setRotation
		//cout << "here" << endl;
	}
	
	int numEntities;
	is >> numEntities;

	for( int i = 0; i < numEntities; ++i )
	{
		Entity *ent = CreateEntity();

		int numActiveFrames;
		is >> numActiveFrames;

		for( int j = 0; j < numActiveFrames; ++j )
		{
			int trueFrame, tsIndex, tsFrame, facingRight, xPos, yPos;
			float rotation;
			float xScale, yScale;
			is >> trueFrame >> tsIndex >> tsFrame >> facingRight 
				>> xPos >> yPos >> rotation >> xScale >> yScale;

			SetEntity( ent, trueFrame, tilesets[tsIndex], tsIndex, facingRight, Vector2f( xPos, yPos ),
				rotation, Vector2f( xScale, yScale ) );
			//ent->GetSp

		}

		//CreateEntity( and set entity

	}

	//have all the tilesets stored

	delete [] tilesets;
}

void SaveToFile()
{
	set<Tileset*> unique;

	int numEntities = allEntities.size();
	for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
	{
		list<SprInfo*> &sList = (*it)->images;
		for( list<SprInfo*>::iterator sit = sList.begin(); sit != sList.end(); ++sit )
		{
			if( (*sit) != NULL )
			{
				unique.insert( (*sit)->ts );
			}
		}
	}

	int numTilesets = unique.size();
	map<Tileset*, int> mapping;
	int index = 0;

	std::ofstream of;
	of.open( fName );

	if( !of.is_open() )
	{
		assert(  "file was invalid" && 0 );
	}
	of << numTilesets << endl;

	for( set<Tileset*>::iterator it = unique.begin(); it != unique.end(); ++it )
	{
		of << (*it)->sourceName
		<< " " << (*it)->tileWidth 
		<< " " << (*it)->tileHeight << endl;

		mapping[(*it)] = index;
		++index;
	}
	
	int numTotalFrames = camera.size();
	of << numTotalFrames << endl;
	for( list<CamInfo>::iterator it = camera.begin(); it != camera.end(); ++it )
	{
		of << (*it).view.getCenter().x << " " << (*it).view.getCenter().y 
			<< " " << (*it).zoomLevel << " " << (*it).angleLevel << endl; 
	}

	of << numEntities << endl;
	for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
	{
		
		list<SprInfo*> &sList = (*it)->images;
		int numActiveFrames = sList.size();
		of << numActiveFrames << endl;

		
		int fIndex = 1;
		for( list<SprInfo*>::iterator sit = sList.begin(); sit != sList.end(); ++sit )
		{
			if( (*sit) == NULL )
			{

			}
			else
			{
				bool facingRight = (*sit)->sprite.getTextureRect().width >= 0;
				Sprite &spr = (*sit)->sprite;
					of << fIndex
					<< " " << mapping[(*sit)->ts]
					<< " " << (*sit)->frame 
					<< " " << (int)facingRight 
					<< " " << spr.getPosition().x << " " << spr.getPosition().y 
					<< " " << spr.getRotation() 
					<< " " << spr.getScale().x << " " << spr.getScale().y << endl;
			}
			

			++fIndex;
		}
	}

	//have a list of all tilesets have assigned them an index
	//int numEntities = 


	//cout << "uniquesize: " << unique.size() << endl;
	//unique.insert( 
}

Entity * MouseDownEntity( int frame, Vector2f mouse )
{
	//cout << "mousex: " << mousex << ", " << mousey << endl;
	Entity *mostFrontEntity = NULL;
	Vector2f point = mouse;
	//Vector2f point( mousex, mousey );
	//Transform trans = camera[frame].getTransform();
	//point = trans.transformPoint( point );
	//Vector2f point = window->mapPixelToCoords(Vector2i( mousex, mousey ));
	//cout << "frame: " << frame << ", point : " << point.x << ", " <<  point.y << endl;
	for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
	{
		if( (*it)->GetSprInfo(frame) != NULL )
		{
			Sprite &spr = (*it)->GetSprInfo(frame)->sprite;
			Vector2f scalePoints[8];
			FloatRect fr = spr.getLocalBounds();

			Vector2f halfSize( fr.width / 2, fr.height / 2 );

			Vector2f topLeft = -halfSize;
			Vector2f topRight = Vector2f( halfSize.x, -halfSize.y );
			Vector2f botRight = halfSize;
			Vector2f botLeft = Vector2f( -halfSize.x, halfSize.y );

			Transform t;
			t.rotate( spr.getRotation() );
			t.scale( spr.getScale() );

			topLeft = t.transformPoint( topLeft ) + spr.getPosition();
			topRight = t.transformPoint( topRight ) + spr.getPosition();
			botRight = t.transformPoint( botRight ) + spr.getPosition();
			botLeft = t.transformPoint( botLeft ) + spr.getPosition();

			scalePoints[0] = topLeft;
			scalePoints[1] = topRight;
			scalePoints[2] = botRight;
			scalePoints[3] = botLeft;


			Vector2f midTop = (topLeft + topRight) / 2.f;
			Vector2f midRight = (topRight + botRight) / 2.f;
			Vector2f midBot = (botLeft + botRight ) / 2.f;
			Vector2f midLeft = (botLeft + topLeft ) / 2.f;

			scalePoints[4] = midTop;
			scalePoints[5] = midRight;
			scalePoints[6] = midBot;
			scalePoints[7] = midLeft;

			bool inScaleCircles = false;
			for( int i = 0; i < 8; ++i )
			{
				if( length( point - scalePoints[i] ) <= transformScaleRadius )
				{
					inScaleCircles = true;
					break;
				}
			}

			bool inRotationCircles = false;
			for( int i = 0; i < 4; ++i )
			{
				Vector2f diff = point - scalePoints[i];
				if( length( diff ) <= transformRotationRadius )
				{
					if( ( i == 0 && diff.x <= 0 && diff.y <= 0 )
						|| ( i == 1 && diff.x >= 0 && diff.y <= 0 )
						|| ( i == 2 && diff.x >= 0 && diff.y >= 0 )
						|| ( i == 3 && diff.x <= 0 && diff.y >= 0 ) )
					{
						inScaleCircles = true;
						break;
					}
				}
			}
			
			//RectangleShape quad;
			/*quad.setSize( Vector2f( fr.width, fr.height ) );
			quad.setScale( spr.getScale() );
			quad.setOrigin( quad.getLocalBounds().width / 2, quad.getLocalBounds().height / 2 );
			quad.setRotation( spr.getRotation() );
			quad.setPosition( spr.getPosition() );

			
			
			RectangleShape rs;*/
			
			//sf::FloatRect fr = spr.getGlobalBounds();
			if( inScaleCircles || inRotationCircles || QuadContainsPoint( topLeft, topRight, botRight, botLeft, point ) )//fr.contains( point ) )
			{
				if( mostFrontEntity == NULL )
				{
					mostFrontEntity = (*it);
				}
				else
				{
					int mfe = mostFrontEntity->layer;
					if( mfe < (*it)->layer )
					{
						mostFrontEntity = (*it);
					}
				}
			}
		}
	}

	return mostFrontEntity;
}

void DrawSelectedEntityBoxes( int frame, sf::RenderTarget *target )
{
	//cout << "Start: " << frame << endl;

	sf::RectangleShape rs;
	//cout << "end" << endl;
	for( list<Entity*>::iterator it = selectedEntities.begin(); it != selectedEntities.end(); ++it )
	{
		//	cout << "End" << endl;
		SprInfo *sprInfo = (*it)->GetSprInfo(frame);
		
		if( sprInfo == NULL )
			continue;
		
		Sprite &spr = sprInfo->sprite;
		FloatRect fr = spr.getLocalBounds();
	
		rs.setRotation( spr.getRotation() );
		rs.setSize( Vector2f( fr.width * spr.getScale().x, fr.height * spr.getScale().y ) );
		rs.setFillColor( Color::Transparent );
		
		rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( spr.getPosition() );
		//rs.setScale( spr.getScale() );
		rs.setOutlineColor( Color::Cyan );
		rs.setOutlineThickness( 3 );
		//
		////Vertex border[ 4 * 4 ];
		///*for( int i = 0; i < 4; ++i )
		//{
		//	Vector2f start 
		//}*/
		////rs.setRotation( spr.getRotation() );
		target->draw( rs );
		/*sf::FloatRect fr = spr.getGlobalBounds();
		rs.setSize( Vector2f( fr.width, fr.height ) );
		rs.setFillColor( Color::Transparent );
		rs.setOutlineColor( Color::Cyan );
		rs.setOutlineThickness( 3 );
		rs.setOrigin( rs.getLocalBounds().width  / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( spr.getPosition().x, spr.getPosition().y );
		target->draw( rs );*/
		Vector2f halfSize( spr.getLocalBounds().width / 2, spr.getLocalBounds().height / 2 );
		Vector2f topLeft = spr.getPosition() - halfSize;
		if( selectedEntities.size() == 1 )
		{
			//Entity *e = selectedEntities.front();
			for( int i = 0; i < 8; ++i )
			{
				if( i % 2 == 0 )
				{

					//border[(i / 2) * 4 + 0].po
				}


				CircleShape &c = transformCircles[i];
				/*c.setFillColor( Color::Green );
				c.setRadius( transformScaleRadius );
				c.setOrigin( c.getLocalBounds().width / 2, c.getLocalBounds().height / 2 );*/
				c.setPosition( transformPoints[i] );
				target->draw( c );
			}
			
			//for( int i = 0; i < 9; ++i )
			//{
			//	int index = i;
			//	if( i == 4 )
			//	{
			//		continue;
			//	}
			//	else if( i > 4 )
			//	{
			//		index--;
			//	}

			//	int x = i % 3;
			//	int y = i / 3;
			//	CircleShape &c = transformCircles[index];
			//	c.setFillColor( Color::Green );
			//	c.setRadius( transformScaleRadius );
			//	c.setOrigin( c.getLocalBounds().width / 2, c.getLocalBounds().height / 2 );

			//	Vector2f fPos( Vector2f( x * halfSize.x, y * halfSize.y ) );
			//	//Transform t = spr.getTransform();
			//	Transform t;
			//	t.rotate( spr.getRotation() );
			//	//cout << "t: " << t.
			//	fPos = t.transformPoint( fPos ) + topLeft;	
			//	c.setPosition( fPos );
			//	target->draw( c );
			//}
		}

	}

	//cout << "true end" << endl;
}

struct Layer
{
	list<Entity*> entities;

};

struct Frame
{

};



void AddLayer()
{

}
list<Layer> layers;

using namespace boost::filesystem;

GUI g;

void SetupEntityType( EntityType* et )
{
	

	int totalNumTiles = 0;
	for( list<Tileset*>::iterator it = et->tSets.begin(); it != et->tSets.end(); ++it )
	{
		Tileset *ts = (*it);
		Vector2u size = ts->texture->getSize(); 
		int xTiles = (size.x / ts->tileWidth);
		int yTiles = (size.y / ts->tileHeight );
		int numTiles = xTiles * yTiles;
		totalNumTiles += numTiles;
	}

	assert( totalNumTiles <= 100 );
	int gridWidth = 12;
	int gridHeight = totalNumTiles / 12 + 1; 
	//cout << "grid height: " << gridHeight << endl;
	//cout << "total num tiles: " << totalNumTiles << endl;

	
	et->panel = new Panel( "test", 500, 500, &g );
	et->panel->pos = Vector2i( 0, 0 );

	et->gs = et->panel->AddGridSelector( "blah", Vector2i( 0, 0 ), gridWidth, gridHeight, 64, 64, true, true );

	
	et->frame = new int*[gridWidth];
	for(int i = 0; i < gridWidth; ++i)
		et->frame[i] = new int[gridHeight];

	/*for( int i = 0; i < gridWidth; ++i )
	{
		for( int j = 0; j < gridHeight; ++j )
		{
			et->frame[i][j] = -1;
		}
	}*/

	int ind = 0;
	for( list<Tileset*>::iterator it = et->tSets.begin(); it != et->tSets.end(); ++it )
	{
		Tileset *ts = (*it);
		Vector2u size = ts->texture->getSize(); 
		int xTiles = (size.x / ts->tileWidth);
		int yTiles = (size.y / ts->tileHeight );
		int numTiles = xTiles * yTiles;
		for( int i = 0; i < numTiles; ++i )
		{
			IntRect ir = ts->GetSubRect( i );
			Sprite sp;
			sp.setTexture( *ts->texture );

			int x = ind % gridWidth;
			int y = ind / gridWidth;

			et->frame[x][y] = i;
			/*if( ts == ts_glide )
			{
				birdInfo[x][y].name = "glide";
			}
			else if( ts == ts_wing )
			{
				birdInfo[x][y].name = "wing";
			}
			else if( ts == ts_kick )
			{
				birdInfo[x][y].name = "kick";
			}
			else if( ts == ts_intro )
			{
				birdInfo[x][y].name = "intro";
			}*/

			//birdInfo[x][y].frame = i;
			sp.setTextureRect( ir );
			sp.setScale( .25, .25 );
			

			//stringstream ss;
			//ss << 
			//cout << "Setting: " << x << ", " << y << endl;
			//string test = ;
			//cout << "source name: " << (*it)->sourceName << endl;
			et->gs->Set( x, y, sp, (*it)->sourceName );
			++ind;
		}
		//int xTiles = 
	}
}

void LoadEntityType( const std::string &fileName, const std::string &fullPath )
{
	//cout << "loading: " << fullPath << ", filename: " << fileName << endl;
	EntityType *et = new EntityType;
	
	et->name = fileName;
	entityTypeList.push_back( et );
	entityTypes[fileName] = et;
	//cout << "load : " << fileName << " .... " << fullPath << endl;
	std::ifstream is;
	is.open( fullPath );
	string rep;
	is >> rep;
	int xtRep, ytRep;
	is >> xtRep;
	is >> ytRep;

	et->ts_rep = GetTileset( rep, xtRep, ytRep );
	//entityTilesets[fileName].push_back( GetTileset( rep, xTile, yTile ) );
	while( !is.eof() )
	{
		string file;
		is >> file;
		int xTile, yTile;
		is >> xTile;
		is >> yTile;

		//entityTilesets[fileName].push_back( GetTileset( file, xTile, yTile ) );
		entityTypes[file] = et;
		et->tSets.push_back( GetTileset( file, xTile, yTile ) );
		//cout << "file: " << file << ", tile: " << xTile << ", " << yTile << endl;
	}

	SetupEntityType( et );
	
}

void LoadEntityTypes()
{
	cout << "gonna load up an entity" << endl;
	path p( current_path() / "Entities" );/// relativePath );
	try
	{
		if (exists(p))    // does p actually exist?
		{
			if (is_regular_file(p))        // is p a regular file?
				cout << p << " size is " << file_size(p) << '\n';

			else if (is_directory(p))      // is p a directory?
			{
			cout << p << " is a directory containing:\n";

				boost::filesystem::directory_iterator it( p );
				//directory_iterator it{p};
				while (it != directory_iterator() ) 
				{
					LoadEntityType( (*it).path().filename().string(), (*it).path().string() );
					++it;
				}
					//std::cout << *it++ << '\n';
			//copy(directory_iterator(p), directory_iterator(),  // directory_iterator::value_type
			//	ostream_iterator<directory_entry>(cout, "\n"));  // is directory_entry, which is
																// converted to a path by the
																// path stream inserter
			}
			else
				cout << p << " exists, but is neither a regular file nor a directory\n";
		}
		else
		{
			cout << p << " does not exist\n";
		}
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << ex.what() << '\n';
	}

	//ifstream is;
	//;;is.open( 
}

void SetEntity( Entity *ent, int frame, Tileset *ts, int tsIndex, bool facingRight, 
	Vector2f &pos, float angle, Vector2f &scale )
{
	//cout << "set entity: " << endl;
	SprInfo *info = ent->GetSprInfo(frame);
	//cout << "detour" << endl;
	if( info == NULL )
	{
		//cout << "detour" << endl;
		list<SprInfo*>::iterator it = ent->GetSprIter( frame );
		(*it) = new SprInfo();
		info = (*it);
	}

	assert( info != NULL );
	//info->
	info->ts = ts;
	info->frame = tsIndex;

	Sprite &tSprite = info->sprite;
	tSprite.setTexture( *ts->texture );
	tSprite.setTextureRect( ts->GetSubRect( tsIndex ) );
	tSprite.setOrigin( tSprite.getLocalBounds().width / 2, tSprite.getLocalBounds().height / 2 );
	tSprite.setPosition( pos );	

	if( !facingRight )
	{
		ent->FlipX( frame );
	}
}

Entity * CreateEntity( /*const std::string &typeName*/ )
{
	//EntityType *et = entityTypes[typeName];

	Entity *ent = new Entity;

	int numFrames = camera.size();
	for( int i = 0; i < numFrames; ++i )
	{
		ent->images.push_back( NULL );
	}

	allEntities.push_back( ent );

	return ent;
}

void DestroyEntity( Entity *ent )
{
	delete ent;
	allEntities.remove( ent );
}

void SetupSelectEntityPanel()
{
	//cout << "Blah " << endl;
	int numTypes = entityTypeList.size();
	int entSelectWidth = 12;
	int entSelectHeight = numTypes / 12 + 1;


	entitySelectPanel = new Panel( "entityselect", entSelectWidth * 64, entSelectHeight * 64, &g );
	GridSelector *esp = entitySelectPanel->AddGridSelector( "blah", Vector2i( 0, 0 ), entSelectWidth, entSelectHeight,
		64, 64, true, true );

	int tind = 0;
	for( list<EntityType*>::iterator it = entityTypeList.begin(); it != entityTypeList.end(); ++it )
	{
		int x = tind % entSelectWidth;
		int y = tind / entSelectWidth;

		Sprite repSprite;
		repSprite.setTexture( *(*it)->ts_rep->texture );
		IntRect ir = (*it)->ts_rep->GetSubRect( 0 );
		repSprite.setTextureRect( ir );
		repSprite.setScale( 64.f / ir.width, 64.f / ir.height );
		esp->Set( x,y , repSprite, (*it)->name );
		//(*it)->
		++tind;
	}
}

int main()
{	
	string testStr;
	
	bool n = false;
	while( true )
	{
		cout << "new (n) or open(o) then press ENTER: ";
		cin >> testStr;
		
		if( testStr == "n" || testStr == "N" )
		{
			n = true;
			break;
		}
		else if( testStr == "o" || testStr == "O" )
		{
			n = false;
			break;
		}
		else
		{
			cout << "error. please correct your input." << endl;
		}
	}
	cout << "file name: ";
	cin >> fName;

	



	creatingEntity = false;
	
	tempCreateMarker.setRadius( 20 );
	tempCreateMarker.setFillColor( Color::White );
	tempCreateMarker.setOrigin( tempCreateMarker.getLocalBounds().width / 2, 
		tempCreateMarker.getLocalBounds().height / 2 );

	showPanel = NULL;
	transformRotationRadius = 40;
	transformScaleRadius = 10;

	/*sf::RectangleShape testr;
	testr.setFillColor( Color::Magenta );
	testr.setSize( Vector2f( 200, 200 ) );
	testr.setPosition( 0, 0 );*/


	//GUI g;
	//birdPanel = new Panel( "test", 500, 500, &g );
	//birdPanel->pos = Vector2i( 0, 0 );
	int gridWidth = 7;
	int gridHeight = 7;

	//GridSelector *gs = birdPanel->AddGridSelector( "blah", Vector2i( 0, 0 ), gridWidth, gridHeight, 64, 64, true, true );
	//showPanel = birdPanel;
	/*Tileset * ts_glide = GetTileset( "Bosses/Bird/glide_256x256.png", 256, 256 );
	Tileset * ts_wing = GetTileset( "Bosses/Bird/wing_256x256.png", 256, 256 );
	Tileset * ts_kick = GetTileset( "Bosses/Bird/kick_256x256.png", 256, 256 );
	Tileset * ts_intro = GetTileset( "Bosses/Bird/intro_256x256.png", 256, 256 );*/


	/*list<Tileset*> birdTilesets;
	birdTilesets.push_back( ts_glide );
	birdTilesets.push_back( ts_wing );
	birdTilesets.push_back( ts_kick );
	birdTilesets.push_back( ts_intro );*/

	for( int i = 0; i < 8; ++i )
	{
		CircleShape &c = transformCircles[i];
		c.setFillColor( Color::Green );
		c.setRadius( transformScaleRadius );
		c.setOrigin( c.getLocalBounds().width / 2, c.getLocalBounds().height / 2 );
		//c.setPosition( transformPoints[i] );
	}

	LoadEntityTypes();

	SetupSelectEntityPanel();

	//camera.push_back( CamInfo() );

	if( n )
	{
		//save empty file
	}
	else
	{
		OpenFromFile( fName );
		//load file from file
	}


	/*for( int i = 0; i < numTypes; ++i )
	{

	}*/

	//showPanel = (*entityTypes.begin()).second->panel;
	
	//gs->Set( 0, 0 Sprite( 
	//Panel *p = new Panel(

	//totalLayers = 10;
	//totalFrames = 10;
	currentFrame = 1;
	currentLayer = 1;

	
	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );

	sf::Text layerText;
	layerText.setFont( arial );
	layerText.setCharacterSize( 18 );
	layerText.setColor( Color::Red );
	layerText.setPosition( 700, 680 );

	sf::Text frameText;
	frameText.setFont( arial );
	frameText.setCharacterSize( 18 );
	frameText.setColor( Color::Green );
	frameText.setPosition( 700, 720 );

	sf::Text camScaleText;
	camScaleText.setFont( arial );
	camScaleText.setCharacterSize( 18 );
	camScaleText.setColor( Color::Yellow );
	camScaleText.setPosition( 700, 760 );

	

	

	//allEntities.front()->images.size();
	//for( int i = 0; i < numFrames; ++i )
	//{
	

	//Entity *testEnt0 = CreateEntity();// "Bird" ); //entityTypes["Bird"]->tSets.front(), 0, true );
	//Entity *testEnt1 = CreateEntity();// "Bird" );// entityTypes["Bird"]->tSets.front(), 0, true );
	//SetEntity( testEnt0, 1, entityTypes["Bird"]->tSets.front(), 0,
	//					true,  Vector2f( 400, 400 ) , 0, Vector2f( 1, 1 ) );
	//SetEntity( testEnt1, 1, entityTypes["Bird"]->tSets.front(), 0,
	//					true,  Vector2f( 100, 100 ) , 0, Vector2f( 1, 1 ) );

		/*CamInfo &ci = camera.back();
		ci.view.setCenter( 0, 0 );
		ci.view.setSize( 1920, 1080 );
		ci.zoomLevel = 40;
		ci.angleLevel = 0;*/
	//}
	//for( int i = 1; i <= numFrames; ++i )
	//{
	//	camera[i].view.setCenter( 0, 0 );
	//	camera[i].view.setSize( 1920, 1080 );
	//	camera[i].zoomLevel = 40;
	//	camera[i].angleLevel = 0;
	//	//cout << "i zoomlevel" << endl;
	//}



	//cout << "how many e" << endl;
	int windowWidth = 1920;
	int windowHeight = 1080;

	View bgView( Vector2f( 0, 0 ), Vector2f( 1920, 1080 ) );
	Tileset *ts_bg = GetTileset( "bg_1_02.png", 1920, 1080 );
	Sprite bgSprite;
	bgSprite.setTexture( *ts_bg->texture );
	bgSprite.setOrigin( bgSprite.getLocalBounds().width / 2, bgSprite.getLocalBounds().height / 2 );

	std::vector<sf::VideoMode> i = sf::VideoMode::getFullscreenModes();
	//window = new sf::RenderWindow( i.front(), "Breakneck", sf::Style::None);
	//sf::VideoMode( windowWidth, windowHeight)
	window = new sf::RenderWindow( i.front(), "Character Animator", sf::Style::None, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	
    sf::CircleShape shape(20.f);
    shape.setFillColor(sf::Color::Green);
	shape.setOrigin( shape.getLocalBounds().width / 2, shape.getLocalBounds().height / 2 );

	mousePressed = false;

	Vector2f mPos;

	

	View uiView;
	
	uiView.setCenter( window->getSize().x / 2, window->getSize().y / 2 );
	//cout << "center: " << uiView.getCenter().x << ", " << uiView.getCenter().y << endl;
	uiView.setSize( window->getSize().x, window->getSize().y );

	int numFrames = camera.size();

    while (window->isOpen())
    {
		View *currView = &GetCamInfo( currentFrame ).view;
		window->clear();
		window->setView( *currView );

        sf::Event ev;

		pastPos = mPos;
		Vector2i mousePos = Mouse::getPosition( *window );
		mPos = window->mapPixelToCoords( mousePos );//Vector2f( mousePos.x, mousePos.y );
		
		
        while (window->pollEvent(ev))
        {
			numFrames = camera.size();//allEntities.front()->images.size();
			int numLayers = allEntities.size();

            if (ev.type == sf::Event::Closed)
                window->close();

			switch( ev.type )
			{
			
			case sf::Event::KeyPressed:
			{
				switch( ev.key.code )
				{
				case Keyboard::Escape:
					return 0;
				case Keyboard::Right:
					if( currentFrame < numFrames )
					{
						++currentFrame;

						if( selectedEntities.size() == 1 )
							UpdateTransformPoints();

						currView = &GetCamInfo( currentFrame ).view;
					}
					break;
				case Keyboard::Left:
					if( currentFrame > 1 )
					{
						--currentFrame;
						if( selectedEntities.size() == 1 )
							UpdateTransformPoints();

						currView = &GetCamInfo( currentFrame ).view;
					}
					break;
				case Keyboard::Up:
					if( currentLayer < numLayers )
					{
						++currentLayer;
					}
					break;
				case Keyboard::Down:
					if( currentLayer > 1 )
					{
						--currentLayer;

						
					}
					break;
				case Keyboard::C:
					if( ev.key.alt )//ev.key.control )
					{
						if( numFrames == 1 )
						{
							break;
						}
						if( currentFrame == numFrames )
						{
							currView = &GetCamInfo( currentFrame - 1 ).view;
						}
						else
						{
							currView = &GetCamInfo( currentFrame ).view;
						}

						RemoveCurrentFrame();
						//cout << "get cam info: " << currentFrame << endl;
						
						numFrames = camera.size();//allEntities.front()->images.size();

						if( selectedEntities.size() == 1 )
							UpdateTransformPoints();
						//cout << "currentFrame : " << currentFrame << endl;
						//copiedEntities.clear();
						//copiedEntities = selectedEntities;
						//copyFrame = currentFrame;
						
					}
					break;
				case Keyboard::V:
					if( ev.key.control )
					{
						/*if( !copiedEntities.empty() )
						{
							for( list<Entity*>::iterator it = copiedEntities.begin(); 
								it != copiedEntities.end(); ++it )
							{
								*(*it)->GetSprInfo(currentFrame) = *(*it)->GetSprInfo(copyFrame);
							}
						}*/
						
					}
					break;
				case Keyboard::B:
					if( ev.key.alt )
					{
						//blank keyframe
						NewBlankFrame();
						currView = &GetCamInfo( currentFrame ).view;
						//cout << "currentFrame : " << currentFrame << endl;
					}
					break;
				case Keyboard::N:
					if( ev.key.alt )
					{
						
						NewCopyFrame();
						currView = &GetCamInfo( currentFrame ).view;
						

						
						//numFrames = allEntities.front()->images.size();
					}
					break;
				case Keyboard::M:
					if( ev.key.alt )
					{
						if( selectedEntities.size() == 1 )
						{
							CopyEntityFrame();
							currView = &GetCamInfo( currentFrame ).view;
						}
					}
					break;
				case Keyboard::F:
					if( selectedEntities.size() == 1 )
					{
						Entity *ent = selectedEntities.front();
						ent->FlipX( currentFrame );
						//ent->SetFaceRight( currentFrame, ent->
					}
					break;
				case Keyboard::S:
					if( ev.key.control )
					{
						SaveToFile();
					}
					break;
				case Keyboard::R:
					if( ev.key.control )
					{
						OpenFromFile( fName );
					}
					break;
				case Keyboard::D:
					if( ev.key.control )
					{
						for( list<Entity*>::iterator it = selectedEntities.begin(); it != selectedEntities.end(); ++it )
						{
							DestroyEntity( (*it) );
						}
						selectedEntities.clear();
					}
					break;
				case Keyboard::E:
					//if( ev.key.control )
					//{
					//	//cout << "show panel is set" << endl;
					//	showPanel = entitySelectPanel;
					//	creatingEntity = true;
					//	//CreateEntity( mPos, 
					//}
					break;
				case Keyboard::PageDown:
					{
						if( Keyboard::isKeyPressed( Keyboard::LShift )
						|| Keyboard::isKeyPressed( Keyboard::RShift ) )
						{
							int fac = 2;
							int &aLevel = GetCamInfo( currentFrame ).angleLevel;
							if( aLevel == 360 / fac - 1 )
							{
								aLevel = 0;
							}
							else
							{
								++aLevel;
							}
							currView->setRotation( aLevel * fac );
						}
						else
						{
							int &zLevel =  GetCamInfo( currentFrame ).zoomLevel;
							if( zLevel < 80 )
							{
								zLevel++;
								currView->setSize( 192 / 4 * zLevel, 108 / 4 * zLevel );
							}
						}
					}
					break;
				case Keyboard::PageUp:
					{
						if( Keyboard::isKeyPressed( Keyboard::LShift )
						|| Keyboard::isKeyPressed( Keyboard::RShift ) )
						{
							int fac = 2;
							int &aLevel =  GetCamInfo( currentFrame ).angleLevel;
							if( aLevel == 0 )
							{
								aLevel = 360 / fac - 1;
							}
							else
							{
								--aLevel;
							}
							currView->setRotation( aLevel * fac );
						}
						else
						{
							int &zLevel =  GetCamInfo( currentFrame ).zoomLevel;
							if( zLevel > 0 )
							{
								zLevel--;
								currView->setSize( 192 / 4 * zLevel, 108 / 4 * zLevel );
							}
						}
						//v.setSize( v.getSize().x - windowWidth * .05, v.getSize().y - windowHeight * .05 );
						//v.setSize( v.getSize().x, v.getSize().y );
					}
					break;
				}

				break;
			}
			case sf::Event::EventType::MouseButtonPressed:
				{
					if( showPanel != NULL )
					{
						showPanel->Update( true, mousePos.x, mousePos.y );
						
						break;
					}
					if( ev.mouseButton.button == sf::Mouse::Button::Left && !entityMove
						&& !entityRotate && !entityScaleX && !entityScaleY && !entityScaleXY )
					{
						mousePressed = true;
						Entity *ent = MouseDownEntity( currentFrame, mPos );
						pressPos = mPos;
						if( ent != NULL  )
						{
							selectedEntities.clear();
							selectedEntities.push_back( ent );

							UpdateTransformPoints();
						//	UpdateSelectedPoints();
							//cout << "selecting" << endl;
							cout << "a" << endl;
						}
						else
						{
							cout << "b" << endl;
							selectedEntities.clear();
						}
					}
					else if( ev.mouseButton.button == sf::Mouse::Button::Middle )
					{
						panning = true;
						panStart = mPos;//currView.getCenter();
					}
					else if( ev.mouseButton.button == sf::Mouse::Button::Right )
					{
						showPanel = entitySelectPanel;
						creatingEntity = true;
						createPos = mPos;
						createPos.x = floor( createPos.x + .5 );
						createPos.y = floor( createPos.y + .5 );
						tempCreateMarker.setPosition( createPos );
						//if( ev.key.control )
					//{
					//	//cout << "show panel is set" << endl;
					//	showPanel = entitySelectPanel;
					//	creatingEntity = true;
					//	//CreateEntity( mPos, 
					//}
					}
					break;
				}
			case sf::Event::MouseButtonReleased:
				{
					if( showPanel != NULL )
					{
						showPanel->Update( false, mousePos.x, mousePos.y );
						break;
					}

					if( ev.mouseButton.button == sf::Mouse::Button::Left )
					{
						if( entityMove )
						{
							for( list<Entity*>::iterator it = selectedEntities.begin();
								it != selectedEntities.end(); ++it )
							{
								SprInfo * sp = (*it)->GetSprInfo( currentFrame ); 
								Vector2f po = sp->sprite.getPosition();
								sp->sprite.setPosition( floor(po.x + .5), floor(po.y + .5 ) );
							}
						}

						if( mousePressed )
						{
							mousePressed = false;
							entityMove = false;
				//			entityMove = false;
							entityRotate = false;
							entityScaleX = false;
							entityScaleY = false;
							entityScaleXY = false;
						}
					}
					else if( ev.mouseButton.button == Mouse::Button::Middle )
					{
						if( panning )
						{
							panning = false;
							CamInfo &ci = GetCamInfo( currentFrame );
							Vector2f oldCenter = ci.view.getCenter();
							ci.view.setCenter( floor(oldCenter.x + .5), floor( oldCenter.y + .5 ) );
						}
					}
					break;
				}
			case sf::Event::MouseWheelMoved:
				{
					CamInfo &ci = GetCamInfo( currentFrame );
					int &zLevel = ci.zoomLevel;
					int &aLevel = ci.angleLevel;
					if( Keyboard::isKeyPressed( Keyboard::LShift )
						|| Keyboard::isKeyPressed( Keyboard::RShift ) )
					{
						int fac = 2;
						if( ev.mouseWheel.delta < 0 )
						{
							if( aLevel == 360 / fac - 1 )
							{
								aLevel = 0;
							}
							else
							{
								++aLevel;
							}
						}
						else if( ev.mouseWheel.delta > 0 )
						{
							if( aLevel == 0 )
							{
								aLevel = 360 / fac - 1;
							}
							else
							{
								--aLevel;
							}
						}

						currView->setRotation( aLevel * fac );
					}
					else
					{
						if( ev.mouseWheel.delta < 0 && zLevel < 80 )
						{
							zLevel++;
							currView->setSize( 192 / 4 * zLevel, 108 / 4 * zLevel );
						}

						else if( ev.mouseWheel.delta > 0 && zLevel > 0 )
						{
							zLevel--;
							currView->setSize( 192 / 4 * zLevel, 108 / 4 * zLevel );
						}
					}
				}
				break;
			}
        }
		
		if( Mouse::isButtonPressed( Mouse::Left ) )
		{
			
			if( !entityMove && !entityRotate && !entityScaleX && !entityScaleY 
				&& !entityScaleXY && mousePressed )
			{	
				if( length( mPos - pressPos ) > 10 )
				{
					entityMove = false;
					entityRotate = false;
					entityScaleX = false;
					entityScaleY = false;
					entityScaleXY = false;

					for( int i = 0; i < 8; ++i )
					{
						Vector2f point = transformPoints[i];
						if( length( pressPos - point ) <= transformScaleRadius )
						{
							selectedPointIndex = i;
							if( i == 0 || i == 2 || i == 5 || i == 7 )
							{
								//cout << "Scale x and y" << endl;
								entityScaleXY = true;
								//entityScaleX = true;
								//entityScaleY = true;
								break;
							}
							else if( i == 3 || i == 4 )
							{
								//cout << "Scale x" << endl;
								entityScaleX = true;
								break;
							}
							else if( i == 1 || i == 6 )
							{
								//cout << "Scale y" << endl;
								entityScaleY = true;
								break;
							}
							//entityRotate = true;
						}
					}

					if( !entityScaleX && !entityScaleY && !entityScaleXY )
					{
						//did not find scaler. time to check rotation
						
						Vector2f topLeft = transformPoints[0];
						Vector2f topRight = transformPoints[2];
						Vector2f botLeft = transformPoints[5];
						Vector2f botRight = transformPoints[7];
						//cout << "top left: " << topLeft.x << " , "<< topLeft.y << endl;
						Vector2f tlDiff = mPos - topLeft;
						Vector2f trDiff = mPos - topRight;
						Vector2f blDiff = mPos - botLeft;
						Vector2f brDiff = mPos - botRight;

						if( length( tlDiff ) <= transformRotationRadius && tlDiff.x <= 0
							&& tlDiff.y <= 0 )
						{
							entityRotate = true;
							//rotatePressAngle =  atan2( tlDiff.y, tlDiff.x );
						}
						else if ( length( trDiff ) <= transformRotationRadius && trDiff.x >= 0
							&& trDiff.y <= 0 )
						{
							entityRotate = true;
							//rotatePressAngle =  atan2( trDiff.y, trDiff.x );
						}
						else if( length( blDiff ) <= transformRotationRadius && blDiff.x <= 0
							&& blDiff.y >= 0 )
						{
							entityRotate = true;
							//rotatePressAngle =  atan2( blDiff.y, blDiff.x );
						}
						else if ( length( brDiff ) <= transformRotationRadius && brDiff.x >= 0
							&& brDiff.y >= 0 )
						{
							entityRotate = true;
							//rotatePressAngle =  atan2( brDiff.y, brDiff.x );
						}


						if( !entityRotate )
						{
							entityMove = true;
						}
						else
						{
							//Vector2f diff = pressPos - sp.
							//rotatePressAngle =  atan2( brDiff.y, brDiff.x );
	
						}
					}

					
					//if( length( mPos - 
					
				}
			}
			else if( entityMove )
			{
				Vector2f diff = mPos - pastPos;
				for( list<Entity*>::iterator it = selectedEntities.begin(); it !=
					selectedEntities.end(); ++it )
				{
					Sprite &sp = (*it)->GetSprInfo(currentFrame)->sprite;
					sp.setPosition( sp.getPosition().x + diff.x, sp.getPosition().y + diff.y );
					
				}

				if( selectedEntities.size() == 1 )
					UpdateTransformPoints();

			}
			else if( entityScaleX )
			{
				//cout << "SCALING X" << endl;
				//cout << "selected entities: " << selectedEntities.size() << endl;
				assert( selectedEntities.size() == 1 );
				Entity *e = selectedEntities.front();

				Sprite &sp = e->GetSprInfo(currentFrame)->sprite;
				float sprAngle = sp.getRotation();

				Vector2f horizAxis( 1, 0 );
				Transform t;
				t.rotate( sp.getRotation() );
				float xDiff = dot( mPos - pastPos, horizAxis );
				
				//sprAngle = sprAngle / 180 * PI;
				//float xDiff = dor( mPos - pastPos, //mPos.x - pastPos.x;

				float width = sp.getLocalBounds().width;
				float hWidth = width / 2;

				float prop = xDiff / hWidth;
				
				if( selectedPointIndex == 3 )
				{
					prop = 1 - prop;
				}
				else 
				{
					prop = 1 + prop;
				}
				
				//cout << "scaling: " << prop << endl;
				sp.scale( prop, 1 );
				UpdateTransformPoints();
				//sp.setScale( prop, sp.getScale().y );
				//sp.setScale( 
				//sp.setPosition( sp.getPosition().x + diff.x, sp.getPosition().y + diff.y );
			}
			else if( entityScaleY )
			{
				assert( selectedEntities.size() == 1 );
				Entity *e = selectedEntities.front();

				Sprite &sp = e->GetSprInfo(currentFrame)->sprite;
				
				Vector2f horizAxis( 1, 0 );
				Transform t;
				t.rotate( sp.getRotation() );

				float yDiff = cross( mPos - pastPos, horizAxis );
				//float yDiff = mPos.y - pastPos.y;

				float height = sp.getLocalBounds().height;
				float hHeight = height / 2;

				float prop = yDiff / hHeight;
				
				if( selectedPointIndex == 1 )
				{
					prop = 1 + prop;
				}
				else 
				{
					prop = 1 - prop;
				}
				
				//cout << "scaling: " << prop << endl;
				sp.scale( 1, prop );
				UpdateTransformPoints();
				//sp.setScale( prop, sp.getScale().y );
				//sp.setScale( 
				//sp.setPosition( sp.getPosition().x + diff.x, sp.getPosition().y + diff.y );
			}
			else if( entityScaleXY )
			{
				assert( selectedEntities.size() == 1 );
				Entity *e = selectedEntities.front();

				Sprite &sp = e->GetSprInfo(currentFrame)->sprite;
				Vector2f horizAxis( 1, 0 );
				Transform t;
				t.rotate( sp.getRotation() );

				float xDiff = dot( mPos - pastPos, horizAxis );
				float yDiff = cross( mPos - pastPos, horizAxis );
				//float xDiff = mPos.x - pastPos.x;
				//float yDiff = mPos.y - pastPos.y;

				float width = sp.getLocalBounds().width;
				float height = sp.getLocalBounds().height;
				float hWidth = width / 2;
				float hHeight = height / 2;

				float propX = xDiff / hWidth;
				float propY = yDiff / hHeight;

				//cout << "selectedPointIndex: " << selectedPointIndex << endl;
				if( selectedPointIndex == 0 || selectedPointIndex == 5 )
				{
					propX = 1 - propX;
				}
				else 
				{
					propX = 1 + propX;
				}

				if( selectedPointIndex == 0 || selectedPointIndex == 2 )
				{
					propY = 1 + propY;
				}
				else 
				{
					propY = 1 - propY;
				}

				sp.scale( propX, propY );
				UpdateTransformPoints();
			}
			else if( entityRotate )
			{
				assert( selectedEntities.size() == 1 );
				Entity *e = selectedEntities.front();
				Sprite &sp = e->GetSprInfo(currentFrame)->sprite;
				Vector2f tDiff = pressPos - sp.getPosition();
				rotatePressAngle = atan2( tDiff.y, tDiff.x );
				Vector2f diff = mPos - sp.getPosition();
				float angle = atan2( diff.y, diff.x );
				//cout << "current angle: " << angle << endl;
				//cout << "rotate press angle: " << rotatePressAngle << endl;

				sp.setRotation( (angle - rotatePressAngle) / PI * 180 );

				UpdateTransformPoints();
				//float originalAngle = 
			}
		}
		if( Mouse::isButtonPressed( Mouse::Middle ) )
		{
			if( panning )
			{

				//Vector2f diff = mPos - pastPos;//mPos - pastPos;

		
				Vector2f temp = panStart - mPos;
				currView->move( temp );
		
				//cout << "diff: " << diff.x << ", " << diff.y << endl;
				//currView.setCenter( currView.getCenter().x - diff.x, currView.getCenter().y - diff.y  );
			}
		}


			

		stringstream ss;
		ss << currentLayer << " / " << allEntities.size();
		layerText.setString( ss.str() );
		ss.str( "" );
		ss.clear();
		ss << currentFrame << " / " << camera.size();
		frameText.setString( ss.str() );
		ss.str("");
		ss.clear();
		Vector2f viewSize = window->getView().getSize();
		

		//ss << "zoom: " << viewSize.x / windowWidth << ", zoomLevel: " << GetCamInfo( currentFrame ).zoomLevel << ", currentFrame: " << currentFrame;
		
		//cout << "ss: " << ss.str() << endl;
		//camScaleText.setString( ss.str() );
		//cout << "blah blah" << endl;
		//ss << currentFrame << " / " << totalFrames << endl;

		//window->setView( bgView );
		//View vv;
		//vv.setCenter( currView.getCenter().x, currV
		//bgSprite.setRotation( currView.getRotation() );
		window->draw( bgSprite );

		//cout << "About to set currview" << endl;
		window->setView( *currView );
		//cout << "currViewSize: " << currView->getSize().x << ", " << currView->getSize().y << endl;
       
		
        window->draw(shape);


		//cout << "just drew shape" << endl;
		for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
		{
			(*it)->Draw( currentFrame, window );
		}
		//cout << "just drew fff" << endl;
		/*if( selectedEntities.size() == 1 )
		{
			UpdateTransformPoints();
		}*/
		DrawSelectedEntityBoxes( currentFrame, window );
		
		if( creatingEntity )
			window->draw( tempCreateMarker );

		window->setView( uiView );
		window->draw( layerText );
		window->draw( frameText );
		window->draw( camScaleText );

		

		if( showPanel != NULL )
			showPanel->Draw( window );
		//birdPanel->Draw( window );
		//window->draw( testr );
		
        window->display();
    }

    return 0;
}