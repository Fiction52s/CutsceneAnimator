#include <SFML/Graphics.hpp>
#include <iostream>
#include <list>
#include <string>
#include "Tileset.h"
#include <sstream>
#include <map>

using namespace std;
using namespace sf;

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
Vector2f pastPos;

float length( Vector2f &v )
{
	return sqrt( v.x * v.x + v.y * v.y );
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
	int layer;
};

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

Entity * MouseDownEntity( int frame, int mousex, int mousey )
{
	cout << "mousex: " << mousex << ", " << mousey << endl;
	Entity *mostFrontEntity = NULL;
	Vector2f point( mousex, mousey );
	for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
	{
		if( (*it)->images.count( frame ) > 0 )
		{
			Sprite &spr = (*it)->images[frame].sprite;
			sf::FloatRect fr = spr.getGlobalBounds();
			if( fr.contains( point ) )
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
		sf::FloatRect fr = spr.getGlobalBounds();
		rs.setSize( Vector2f( fr.width, fr.height ) );
		rs.setFillColor( Color::Transparent );
		rs.setOutlineColor( Color::Cyan );
		rs.setOutlineThickness( 3 );
		rs.setOrigin( rs.getLocalBounds().width  / 2, rs.getLocalBounds().height / 2 );
		rs.setPosition( spr.getPosition().x, spr.getPosition().y );
		target->draw( rs );
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
	sf::Font arial;
	arial.loadFromFile( "arial.ttf" );

	sf::Text layerText;
	layerText.setFont( arial );
	layerText.setCharacterSize( 18 );
	layerText.setColor( Color::Red );
	layerText.setPosition( 700, 700 );

	sf::Text frameText;
	frameText.setFont( arial );
	frameText.setCharacterSize( 18 );
	frameText.setColor( Color::Green );
	frameText.setPosition( 700, 750 );

	totalLayers = 10;
	totalFrames = 10;
	currentFrame = 1;
	currentLayer = 1;

	Tileset * ts_glide = GetTileset( "Bosses/Bird/glide_256x256.png", 256, 256 );
	Tileset * ts_wing = GetTileset( "Bosses/Bird/wing_256x256.png", 256, 256 );
	Tileset * ts_kick = GetTileset( "Bosses/Bird/kick_256x256.png", 256, 256 );
	Tileset * ts_intro = GetTileset( "Bosses/Bird/intro_256x256.png", 256, 256 );
	
	Entity *test = new Entity;
	Sprite &tSprite = test->images[1].sprite;
	tSprite.setTexture( *ts_kick->texture );
	tSprite.setTextureRect( ts_kick->GetSubRect( 5 ) );
	tSprite.setOrigin( tSprite.getLocalBounds().width / 2, tSprite.getLocalBounds().height / 2 );
	tSprite.setPosition( 400, 400 );

	allEntities.push_back( test );
	//cout << "how many e" << endl;
    sf::RenderWindow window(sf::VideoMode(800, 800), "SFML works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

	mousePressed = false;

	Vector2f mPos;

    while (window.isOpen())
    {
        sf::Event ev;

		pastPos = mPos;
		Vector2i mousePos = Mouse::getPosition( window );
		mPos = Vector2f( mousePos.x, mousePos.y );
		

        while (window.pollEvent(ev))
        {
            if (ev.type == sf::Event::Closed)
                window.close();

			switch( ev.type )
			{
			
			case sf::Event::KeyPressed:
			{
				switch( ev.key.code )
				{
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
				}

				break;
			}
			case sf::Event::EventType::MouseButtonPressed:
				{
					if( ev.mouseButton.button == sf::Mouse::Button::Left && !entityMove )
					{
						mousePressed = true;
						Entity *ent = MouseDownEntity( currentFrame, mPos.x, mPos.y );
						pressPos = mPos;
						if( ent != NULL  )
						{
							selectedEntities.clear();
							selectedEntities.push_back( ent );
							
							//cout << "selecting" << endl;
						}
						else
						{
							selectedEntities.clear();
						}
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
						}
					}
					break;
				}
			}
        }

		if( Mouse::isButtonPressed( Mouse::Left ) )
		{
			
			if( !entityMove && mousePressed )
			{	
				if( length( mPos - pressPos ) > 10 )
				{
					entityMove = true;
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

			}
		}

		stringstream ss;
		ss << currentLayer << " / " << totalLayers << endl;
		layerText.setString( ss.str() );
		ss.str( "" );
		ss.clear();
		ss << currentFrame << " / " << totalFrames << endl;
		frameText.setString( ss.str() );


        window.clear();
        window.draw(shape);
		window.draw( layerText );
		window.draw( frameText );
		for( list<Entity*>::iterator it = allEntities.begin(); it != allEntities.end(); ++it )
		{
			(*it)->Draw( currentFrame, &window );
		}
		//test.Draw( currentFrame, &window );
		DrawSelectedEntityBoxes( currentFrame, &window );
        window.display();
    }

    return 0;
}