#include <SFML/Graphics.hpp>
#include <iostream>
#include <list>
#include <string>
#include "Tileset.h"
#include <sstream>
#include <assert.h>
#include <map>
#include "GUI.h"

using namespace std;
using namespace sf;

#define PI 3.14159

struct Entity;
//const int NUM_FRAMES = 60;

list<Tileset*> tilesetList;
list<Entity*> selectedEntities;
list<Entity*> allEntities;

int currentFrame;
int currentLayer;
int totalLayers;
int totalFrames;
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

float rotatePressAngle;
sf::CircleShape transformCircles[8];
Vector2f transformPoints[8];

int transformRotationRadius;
int transformScaleRadius;


bool panning;
Vector2f panStart;

struct GUI : GUIHandler
{
	void ButtonCallback( Button *b, const std::string & e )
	{
	}

	void TextBoxCallback( TextBox *tb, const std::string & e )
	{
	}
	
	void GridSelectorCallback( GridSelector *gs, const std::string & e )
	{
	}

	void CheckBoxCallback( CheckBox *cb, const std::string & e )
	{
	}
};

struct CamInfo
{
	View view;
	int zoomLevel;
	int angleLevel;
};

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

map<int, CamInfo> camera;
sf::RenderWindow *window;



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
	}
	Sprite sprite;
	bool facingRight;
	bool selected;
};

struct Entity
{
	void SetFaceRight( int frame, bool right );
	void Draw( int frame, sf::RenderTarget *target );

	map<int,SprInfo> images;
	Vector2f GetPoint( int index );
	int layer;
};


//Vector2f Entity::GetPoint( int frame, int index )
void UpdateTransformPoints()
{
	assert( selectedEntities.size() == 1 );
	Entity *e = selectedEntities.front();
	
	Sprite &spr = e->images[currentFrame].sprite;
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
	target->draw( images[frame].sprite );
}

void Entity::SetFaceRight( int frame, bool right )
{
	SprInfo oldInfo = images[frame];
	//bool &old = images[frame].facingRight;
	if( oldInfo.facingRight != right )
	{
		sf::IntRect r = oldInfo.sprite.getTextureRect();
		oldInfo.sprite.setTextureRect( sf::IntRect( r.left + r.width, r.top, -r.width, r.height ) );
		oldInfo.facingRight = right;
		//facingRight = right;
	}
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
		if( (*it)->images.count( frame ) > 0 )
		{
			Sprite &spr = (*it)->images[frame].sprite;
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
	sf::RectangleShape rs;
	for( list<Entity*>::iterator it = selectedEntities.begin(); it != selectedEntities.end(); ++it )
	{
		Sprite &spr = (*it)->images[frame].sprite;
		FloatRect fr = spr.getLocalBounds();

		rs.setRotation( spr.getRotation() );
		rs.setSize( Vector2f( fr.width * spr.getScale().x, fr.height * spr.getScale().y ) );
		rs.setFillColor( Color::Transparent );
		
		rs.setOrigin( rs.getLocalBounds().width / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( spr.getPosition() );
		//rs.setScale( spr.getScale() );
		rs.setOutlineColor( Color::Cyan );
		rs.setOutlineThickness( 3 );

		//Vertex border[ 4 * 4 ];
		/*for( int i = 0; i < 4; ++i )
		{
			Vector2f start 
		}*/
		//rs.setRotation( spr.getRotation() );
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

int main()
{	
	transformRotationRadius = 40;
	transformScaleRadius = 10;

	GUI g;
	Panel *p = new Panel( "test", 500, 500, &g );
	GridSelector *gs = p->AddGridSelector( "blah", Vector2i( 0, 0 ), 5, 5, 64, 64, true, true );

	Tileset * ts_glide = GetTileset( "Bosses/Bird/glide_256x256.png", 256, 256 );
	Tileset * ts_wing = GetTileset( "Bosses/Bird/wing_256x256.png", 256, 256 );
	Tileset * ts_kick = GetTileset( "Bosses/Bird/kick_256x256.png", 256, 256 );
	Tileset * ts_intro = GetTileset( "Bosses/Bird/intro_256x256.png", 256, 256 );
	list<Tileset*> birdTilesets;
	birdTilesets.push_back( ts_glide );
	birdTilesets.push_back( ts_wing );
	birdTilesets.push_back( ts_kick );
	birdTilesets.push_back( ts_intro );

	for( int i = 0; i < 8; ++i )
	{
		CircleShape &c = transformCircles[i];
		c.setFillColor( Color::Green );
		c.setRadius( transformScaleRadius );
		c.setOrigin( c.getLocalBounds().width / 2, c.getLocalBounds().height / 2 );
		//c.setPosition( transformPoints[i] );
	}

	for( list<Tileset*>::iterator it = birdTilesets.begin(); it != birdTilesets.end(); ++it )
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
			sp.setTextureRect( ir );
			sp.setScale( .25, .25 );
			int x = i % xTiles;
			int y = i / xTiles;

			//stringstream ss;
			//ss << 
			cout << "Setting: " << x << ", " << y << endl;
			gs->Set( x, y, sp, "blah" );
		}
		//int xTiles = 
	}
	//gs->Set( 0, 0 Sprite( 
	//Panel *p = new Panel(

	totalLayers = 10;
	totalFrames = 10;
	currentFrame = 1;
	currentLayer = 1;

	for( int i = 1; i < totalFrames; ++i )
	{
		camera[i].view.setCenter( 0, 0 );
		camera[i].view.setSize( 1920, 1080 );
		camera[i].zoomLevel = 40;
		camera[i].angleLevel = 0;
		//cout << "i zoomlevel" << endl;
	}

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

	

	
	
	Entity *test = new Entity;
	Sprite &tSprite = test->images[1].sprite;
	tSprite.setTexture( *ts_kick->texture );
	tSprite.setTextureRect( ts_kick->GetSubRect( 5 ) );
	tSprite.setOrigin( tSprite.getLocalBounds().width / 2, tSprite.getLocalBounds().height / 2 );
	tSprite.setPosition( 400, 400 );

	allEntities.push_back( test );
	//cout << "how many e" << endl;
	int windowWidth = 1920;
	int windowHeight = 1080;

	View bgView( Vector2f( 0, 0 ), Vector2f( 1920, 1080 ) );
	Tileset *ts_bg = GetTileset( "bg_1_02.png", 1920, 1080 );
	Sprite bgSprite;
	bgSprite.setTexture( *ts_bg->texture );
	bgSprite.setOrigin( bgSprite.getLocalBounds().width / 2, bgSprite.getLocalBounds().height / 2 );

	
    window = new sf::RenderWindow( sf::VideoMode( windowWidth, windowHeight), "Character Animator", sf::Style::Default, sf::ContextSettings( 0, 0, 0, 0, 0 ));
	
    sf::CircleShape shape(20.f);
    shape.setFillColor(sf::Color::Green);
	shape.setOrigin( shape.getLocalBounds().width / 2, shape.getLocalBounds().height / 2 );

	mousePressed = false;

	Vector2f mPos;

	View uiView;
	uiView.setCenter( window->getSize().x / 2, window->getSize().y / 2 );
	uiView.setSize( window->getSize().x, window->getSize().y );

    while (window->isOpen())
    {
		View &currView = camera[currentFrame].view;
		window->clear();
		window->setView( currView );

        sf::Event ev;

		pastPos = mPos;
		Vector2i mousePos = Mouse::getPosition( *window );
		mPos = window->mapPixelToCoords( mousePos );//Vector2f( mousePos.x, mousePos.y );
		

        while (window->pollEvent(ev))
        {
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
					if( currentFrame < totalFrames )
					{
						++currentFrame;
					}
					break;
				case Keyboard::Left:
					if( currentFrame > 1 )
					{
						--currentFrame;
					}
					break;
				case Keyboard::Up:
					if( currentLayer < totalLayers )
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
					if( ev.key.control )
					{
						//copiedEntities.clear();
						copiedEntities = selectedEntities;
						copyFrame = currentFrame;
						
					}
					break;
				case Keyboard::V:
					if( ev.key.control )
					{
						if( !copiedEntities.empty() )
						{
							for( list<Entity*>::iterator it = copiedEntities.begin(); 
								it != copiedEntities.end(); ++it )
							{
								(*it)->images[currentFrame] = (*it)->images[copyFrame];
							}
						}
						
					}
					break;
				case Keyboard::PageDown:
					{
						if( Keyboard::isKeyPressed( Keyboard::LShift )
						|| Keyboard::isKeyPressed( Keyboard::RShift ) )
						{
							int fac = 2;
							int &aLevel = camera[currentFrame].angleLevel;
							if( aLevel == 360 / fac - 1 )
							{
								aLevel = 0;
							}
							else
							{
								++aLevel;
							}
							currView.setRotation( aLevel * fac );
						}
						else
						{
							int &zLevel = camera[currentFrame].zoomLevel;
							if( zLevel < 80 )
							{
								zLevel++;
								currView.setSize( 192 / 4 * zLevel, 108 / 4 * zLevel );
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
							int &aLevel = camera[currentFrame].angleLevel;
							if( aLevel == 0 )
							{
								aLevel = 360 / fac - 1;
							}
							else
							{
								--aLevel;
							}
							currView.setRotation( aLevel * fac );
						}
						else
						{
							int &zLevel = camera[currentFrame].zoomLevel;
							if( zLevel > 0 )
							{
								zLevel--;
								currView.setSize( 192 / 4 * zLevel, 108 / 4 * zLevel );
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
					break;
				}
			case sf::Event::MouseButtonReleased:
				{
					if( ev.mouseButton.button == sf::Mouse::Button::Left )
					{
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
						}
					}
					break;
				}
			case sf::Event::MouseWheelMoved:
				{

					int &zLevel = camera[currentFrame].zoomLevel;
					int &aLevel = camera[currentFrame].angleLevel;
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

						currView.setRotation( aLevel * fac );
					}
					else
					{
						if( ev.mouseWheel.delta < 0 && zLevel < 80 )
						{
							zLevel++;
							currView.setSize( 192 / 4 * zLevel, 108 / 4 * zLevel );
						}

						else if( ev.mouseWheel.delta > 0 && zLevel > 0 )
						{
							zLevel--;
							currView.setSize( 192 / 4 * zLevel, 108 / 4 * zLevel );
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
					Sprite &sp = (*it)->images[currentFrame].sprite;
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

				Sprite &sp = e->images[currentFrame].sprite;
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

				Sprite &sp = e->images[currentFrame].sprite;
				
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

				Sprite &sp = e->images[currentFrame].sprite;
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
				Sprite &sp = e->images[currentFrame].sprite;
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
				currView.move( temp );
		
				//cout << "diff: " << diff.x << ", " << diff.y << endl;
				//currView.setCenter( currView.getCenter().x - diff.x, currView.getCenter().y - diff.y  );
			}
		}

		stringstream ss;
		ss << currentLayer << " / " << totalLayers;
		layerText.setString( ss.str() );
		ss.str( "" );
		ss.clear();
		ss << currentFrame << " / " << totalFrames;
		frameText.setString( ss.str() );
		ss.str("");
		ss.clear();
		Vector2f viewSize = window->getView().getSize();
		ss << "zoom: " << viewSize.x / windowWidth << ", zoomLevel: " << camera[currentFrame].zoomLevel << ", currentFrame: " << currentFrame;
		camScaleText.setString( ss.str() );
		//ss << currentFrame << " / " << totalFrames << endl;

		//window->setView( bgView );
		//View vv;
		//vv.setCenter( currView.getCenter().x, currV
		//bgSprite.setRotation( currView.getRotation() );
		window->draw( bgSprite );
		window->setView( currView );
       

        window->draw(shape);

		for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
		{
			(*it)->Draw( currentFrame, window );
		}

		/*if( selectedEntities.size() == 1 )
		{
			UpdateTransformPoints();
		}*/
		DrawSelectedEntityBoxes( currentFrame, window );

		window->setView( uiView );
		window->draw( layerText );
		window->draw( frameText );
		window->draw( camScaleText );
		//p->Draw( window );

        window->display();
    }

    return 0;
}