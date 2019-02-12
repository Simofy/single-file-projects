

//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string>
//#include <cmath>
#include <windows.h>

#include <sstream>
#include <math.h>
//#include <iostream>

#define PI 3.14159265
namespace patch
{
template < typename T > std::string to_string( const T& n )
{
    std::ostringstream stm ;
    stm << n ;
    return stm.str() ;
}
}
//Screen dimension constants
const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 640;

const int TOTAL_MENU_BUTTONS = 4;
const int TOTAL_TREE_BUTTONS = 3;
SDL_Rect cursor_plain = {0,0,25,18};
SDL_Rect cursor_question = {25,0,25,18};
SDL_Rect cursor_click = {50,0,25,18};
SDL_Rect cursor_pan = {75,0,25,18};
SDL_Rect cursor_pan_select = {100,0,25,18};
SDL_Rect cursor_text = {125,0,25,18};
SDL_Rect cursor_size_y = {150,0,25,18};
SDL_Rect cursor_size_x = {175,0,25,18};


SDL_Rect size_slider = {0,0,0,40};
SDL_Rect angle_slider = {0,0,0,60};
SDL_Rect width_slider = {0,0,0,120};
SDL_Rect speed_slider = {0,0,0,12};
SDL_Texture* loadTexture( std::string path );

SDL_Color textColor = { 0, 0, 0, 0xFF };
//The window we'll be rendering to
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
//The surface contained by the window
SDL_Surface* gScreenSurface = NULL;
SDL_Texture* MainLook = NULL;
//The image we will load and show on the screen
SDL_Surface* gXOut = NULL;
//The window we'll be rendering to
//Globally used font
TTF_Font *gFont = NULL;
TTF_Font *gFontB = NULL;
//Scene textures
struct Color
{
    int r,g,b;
    UINT8 a;
};

struct Map
{
    float x,y,scale,temp;
} map,mouseMove,ref;


//_______________________________________________________________________________________
class LTexture
{
public:
    //Initializes variables
    LTexture();

    //Deallocates memory
    ~LTexture();

    //Loads image at specified path
    bool loadFromFile( std::string path );

    //Creates image from font string
    bool loadFromRenderedText( std::string textureText, TTF_Font *font,SDL_Color textColor );


    //Deallocates texture
    void free();

    //Set color modulation
    void setColor( Uint8 red, Uint8 green, Uint8 blue );

    //Set blending
    void setBlendMode( SDL_BlendMode blending );

    //Set alpha modulation
    void setAlpha( Uint8 alpha );

    //Renders texture at given point
    void render( int x, int y, float scale,SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

    //Gets image dimensions
    int getWidth();
    int getHeight();

private:
    //The actual hardware texture
    SDL_Texture* mTexture;

    //Image dimensions
    int mWidth;
    int mHeight;
};
struct node
{
    int step;
    node *left, *right;
    int key_value;
    LTexture Tvalue;
    bool animate;
} *root;
LTexture::LTexture()
{
    //Initialize
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

LTexture::~LTexture()
{
    //Deallocate
    free();
}

bool LTexture::loadFromFile( std::string path )
{
    //Get rid of preexisting texture
    free();

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Color key image
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    //Return success
    mTexture = newTexture;
    return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( std::string textureText, TTF_Font *font, SDL_Color textColor )
{
    //Get rid of preexisting texture
    free();

    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( font, textureText.c_str(), textColor );
    if( textSurface != NULL )
    {
        //Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }
    else
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }


    //Return success
    return mTexture != NULL;
}
#endif


void LTexture::free()
{
    //Free texture if it exists
    if( mTexture != NULL )
    {
        SDL_DestroyTexture( mTexture );
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
    //Modulate texture rgb
    SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
    //Set blending function
    SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
    //Modulate texture alpha
    SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, float scale, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
    //Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y, mWidth*scale, mHeight*scale };

    //Set clip rendering dimensions
    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    //Render to screen
    SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}
//_______________________________________________________________________________________





Color BUTTON_SPRITE_MOUSE_OUT = { 240, 240, 240, 0xFF};
Color BUTTON_SPRITE_MOUSE_OVER_MOTION = { 216, 230, 242, 0xFF};
Color BUTTON_SPRITE_MOUSE_DOWN = {204,204,204,0xFF};
Color BUTTON_SPRITE_MOUSE_UP = {193,220,243,0xFF};


//Texture wrapper class
class Lbutton
{
public:
    bool loadFromFile( std::string path);
    bool buttonName( std::string textureText,TTF_Font *font, SDL_Color textColor );
    bool clicked;
    LButton();
    //Sets top left position
    void setPosition( int x, int y );
    //Handles mouse event
    void handleEvent( SDL_Event* e, SDL_Rect ref,int i, int *selected);
    //Shows button sprite
    void render(int nr, int screen);
    int BUTTON_WIDTH;
    int BUTTON_HEIGHT;
    bool setMeasurements(int w, int h)
    {
        BUTTON_WIDTH = w;
        BUTTON_HEIGHT = h;
    }
private:
    void render( int x, int y, Color color)
    {
        //Set rendering space and render to screen
        SDL_Rect renderQuad = { x, y, BUTTON_WIDTH, BUTTON_HEIGHT};
        SDL_SetRenderDrawColor( gRenderer, color.r, color.g, color.b,color.a );
        SDL_RenderFillRect( gRenderer, &renderQuad );
        SDL_SetRenderDrawColor( gRenderer, 130, 135, 144, 0xFF );
        SDL_RenderDrawRect( gRenderer, &renderQuad );
        //SDL_RenderCopyEx( gRenderer, mTexture, NULL, NULL, NULL, NULL, NULL );
        SDL_Rect* clip1;
        double angle;
        SDL_Point* center;
        SDL_RendererFlip flip;
        renderQuad = { (x+BUTTON_WIDTH/2-mWidth/2), (y+BUTTON_HEIGHT/2-mHeight/2), mWidth, mHeight };
        SDL_RenderCopyEx( gRenderer, bTextTexture, clip1, &renderQuad, angle, center, flip );
        //____________________________________
    }

    std::string buttonN;
    int satus;
    SDL_Texture* mTexture;
    SDL_Texture* bTextTexture;

    //Image dimensions
    int mWidth;
    int mHeight;
    SDL_Point mPosition;
    //Currently used global sprite
    Color mCurrentSprite;

} buttonMenu[TOTAL_MENU_BUTTONS],buttonTree[TOTAL_TREE_BUTTONS];

bool Lbutton::loadFromFile( std::string path)
{
    //Get rid of preexisting texture
    //free();

    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Color key image
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    //Return success
    mTexture = newTexture;
    return mTexture!= NULL;
}
bool Lbutton::buttonName( std::string textureText,TTF_Font *font, SDL_Color textColor )
{
    //Get rid of preexisting texture
    //free();

    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( font, textureText.c_str(), textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        bTextTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        //Get image dimensions
        mWidth = textSurface->w;
        mHeight = textSurface->h;
        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }

    //Return success
    return mTexture != NULL;
}


Lbutton::LButton()
{
    mPosition.x = 0;
    mPosition.y = 0;
    mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
    clicked = false;
}
void Lbutton::setPosition( int x, int y )
{
    mPosition.x = x;
    mPosition.y = y;
}
void Lbutton::handleEvent( SDL_Event* e, SDL_Rect ref,int i, int *selected)
{
    //If mouse event happened
    if( e->type == SDL_MOUSEMOTION || e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP )
    {

        //Get mouse position
        int x, y;
        SDL_GetMouseState( &x, &y );

        //Check if mouse is in button
        bool inside = true;



        //Mouse is left of the button
        if( x-ref.x < mPosition.x )
        {
            inside = false;
        }
        //Mouse is right of the button
        else if( x-ref.x > mPosition.x + BUTTON_WIDTH )
        {
            inside = false;
        }
        //Mouse above the button
        else if( y-ref.y < mPosition.y )
        {
            inside = false;
        }
        //Mouse below the button
        else if( y-ref.y > mPosition.y + BUTTON_HEIGHT )
        {
            inside = false;
        }
        //Mouse is outside button
        if( !inside && !clicked)
        {
            mCurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
        }
        //Mouse is inside button
        else
        {
            if(!clicked && e->type == SDL_MOUSEMOTION)
                mCurrentSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
            if(!clicked && e->type == SDL_MOUSEBUTTONUP && mCurrentSprite.b == BUTTON_SPRITE_MOUSE_DOWN.b && mCurrentSprite.g == BUTTON_SPRITE_MOUSE_DOWN.g&& mCurrentSprite.r == BUTTON_SPRITE_MOUSE_DOWN.r)
            {
                mCurrentSprite = BUTTON_SPRITE_MOUSE_UP;
                clicked = true;
                *selected = i+1;
            }


            if(!clicked && e->type == SDL_MOUSEBUTTONDOWN)
                mCurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
            /*
            //Set mouse over sprite
            switch( e->type )
            {
            	case SDL_MOUSEMOTION:
            	mCurrentSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
            	break;

            	case SDL_MOUSEBUTTONDOWN:
            	mCurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
            	break;

            	case SDL_MOUSEBUTTONUP:
            	mCurrentSprite = BUTTON_SPRITE_MOUSE_UP;
            	break;
            }
            //*/
        }
    }
}
SDL_Rect Dcolor;
void Lbutton::render(int nr, int screen)
{
    //Show current button sprite
    switch (screen)
    {
    case 1:
        buttonMenu[nr].render( mPosition.x, mPosition.y, mCurrentSprite);
        break;
    case 3:
        buttonTree[nr].render( mPosition.x, mPosition.y, mCurrentSprite);
        break;
    }

}

SDL_Texture* loadTexture( std::string path )
{
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return newTexture;
}

bool init()
{
    bool success = true;
    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        {
            printf( "Warning: Linear texture filtering not enabled!" );
        }
        //Create window
        gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN  );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Get window surface
            gScreenSurface = SDL_GetWindowSurface( gWindow );
            //Create renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                SDL_SetWindowResizable(gWindow,SDL_TRUE);
                //Initialize renderer color
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
            }
        }
    }

    return success;
}
LTexture textTest;
LTexture node_tree;
LTexture tree_button;
//LTexture treeButton[3];


void buttonUpdate (double w, double h, bool layout, int total, Lbutton *button )
{
    int k = 0;
    for(int  i = 0; i < total; i++ )
    {
        button[ i ].setMeasurements(w,h);
        if(layout!=false)
        {
            button[ i ].setPosition( 0, k );
            k+=h+4;
        }
        else
        {
            button[ i ].setPosition( k, 0 );
            k+=w+4;
        }
    }
}

LTexture cursor;

LTexture gPromptTextTexture;
LTexture gInputTextTexture;

LTexture windowOptionText[4];
LTexture windowCreditText[1];
bool loadMedia()
{
    cursor.loadFromFile("cursor.png");
    //Set sprites
    bool success = true;
    int k=SCREEN_HEIGHT*0.1;

    //buttonUpdate(SCREEN_WIDTH*0.3-12,SCREEN_HEIGHT*0.05,true,TOTAL_MENU_BUTTONS,buttonMenu);
    //buttonUpdate(SCREEN_WIDTH*0.1-12,SCREEN_HEIGHT*0.05,false,TOTAL_TREE_BUTTONS,buttonTree);

    // reikia is failo kad skaitytu
    if( tree_button.loadFromFile( "tree_button.png" ) == NULL )
    {
        printf( "Failed to load texture image!\n" );
        success = false;
    }
    gFontB = TTF_OpenFont( "fonts/OpenSans-Regular.ttf", 12 );
    if( gFontB == NULL )
    {
        printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    else
    {
        //Render text
        buttonTree[0].buttonName("Insert",gFontB,textColor);
        buttonTree[1].buttonName("Search",gFontB,textColor);
        buttonTree[2].buttonName("Delete",gFontB,textColor);
        //
        buttonMenu[0].buttonName("Naujas medis",gFontB,textColor);
        buttonMenu[1].buttonName("Options",gFontB,textColor);
        buttonMenu[2].buttonName("Save",gFontB,textColor);
        buttonMenu[3].buttonName("Exit",gFontB,textColor);
        windowOptionText[0].loadFromRenderedText("Otions",gFontB,textColor);
        windowOptionText[1].loadFromRenderedText("Size",gFontB,textColor);
        windowOptionText[2].loadFromRenderedText("Angle",gFontB,textColor);
        windowOptionText[3].loadFromRenderedText("Recursive angle",gFontB,textColor);
        windowCreditText[0].loadFromRenderedText("Credits",gFontB,textColor);
        //SDL_Color textColor = { 0, 0, 0, 0xFF };

    }

    gFont = TTF_OpenFont( "fonts/OpenSans-Bold.ttf", 96 );
    if( gFont == NULL )
    {
        printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }

    if( node_tree.loadFromFile( "node_tree.png" ) == NULL )
    {
        printf( "Failed to load texture image!\n" );
        success = false;
    }
    SDL_Color textColor = { 0, 0, 0, 0xFF };
    if( !gPromptTextTexture.loadFromRenderedText( "Enter number:",gFontB, textColor ) )
    {
        printf( "Failed to render prompt text!\n" );
        success = false;
    }
    return success;
}




void close()
{
    //Deallocate surface
    SDL_ShowCursor(SDL_ENABLE);
    SDL_FreeSurface( gXOut );
    gXOut = NULL;

    //Destroy wiow
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;

    //Quit SDL subsystems
    SDL_Quit();
}

class btree
{
public:
    btree();
    //~btree();
    void PreorderRender();
    void insert(int key);
    node *search(int key);
    void Delete(int key);
    void destroy_tree();


private: // reikia ikelt textura cia
    node* FindMin(node* root);
    int timer;
    void Preorder(node* leaf, int dir, float x, float y, int rad);
    void destroy_tree(node *leaf);
    void insert(int key, node *leaf);
    node* Delete(node *root, int key);
    void animate(node* leaf);
    node *search(int key, node *leaf);
    node *root;
};
btree::btree()
{
    timer = 0;
    root=NULL;
}
void btree::PreorderRender()
{
    Preorder(root, 0, map.x, map.y, angle_slider.h-12);
}
void btree::destroy_tree(node *leaf)
{
    if(leaf!=NULL)
    {
        destroy_tree(leaf->left);
        destroy_tree(leaf->right);
        delete leaf;
    }
}
void btree::insert(int key, node *leaf)
{
    if(key< leaf->key_value)
    {
        if(leaf->left!=NULL)
        {
            insert(key, leaf->left);
        }
        else
        {

            leaf->left=new node;
            leaf->left->key_value=key;
            leaf->left->left=NULL;    //Sets the left child of the child node to null
            leaf->left->right=NULL;   //Sets the right child of the child node to null
            std::string value =  patch::to_string(key);
            leaf->left->Tvalue.loadFromRenderedText(value,gFont,textColor);
            leaf->left->animate = true;
            leaf->left->step = 0;
        }
    }
    else if(key>=leaf->key_value)
    {
        if(leaf->right!=NULL)
        {
            insert(key, leaf->right);
        }

        else
        {
            leaf->right=new node;
            leaf->right->key_value=key;
            leaf->right->left=NULL;  //Sets the left child of the child node to null
            leaf->right->right=NULL; //Sets the right child of the child node to null
            std::string value =  patch::to_string(key);
            leaf->right->Tvalue.loadFromRenderedText(value,gFont,textColor);
            leaf->right->animate = true;
            leaf->right->step = 0;
        }
    }
}
node *btree::search(int key, node *leaf)
{
    if(leaf!=NULL)
    {
        if(key==leaf->key_value)
            return leaf;
        if(key<leaf->key_value)
            return search(key, leaf->left);
        else
            return search(key, leaf->right);
    }
    else return NULL;
}
void btree::insert(int key)
{
    if(root!=NULL)
        insert(key, root);
    else
    {
        root=new node;
        root->key_value=key;
        root->left=NULL;
        root->right=NULL;
        std::string value =  patch::to_string(key);
        root->Tvalue.loadFromRenderedText(value, gFont,textColor);
        root->animate = false;
        root->step = 0;
    }
}
void btree::Delete(int key)
{
    Delete(root, key);
}
node *btree::search(int key)
{
    return search(key, root);
}
void btree::destroy_tree()
{
    destroy_tree(root);
}
//padaryti GAL i kaire juostos kitos spalvos nei i desine
void btree::animate(node* leaf)
{
    int size = size_slider.h;
    if(timer<10)
        timer++;
    else
    {
        leaf->step+=1;
        timer = 0;
        if(leaf->step>=size)leaf->animate = false;
    }


}
void btree::Preorder(node* leaf, int dir, float x, float y, int rad)
{
    float x0;
    float y0;
    int deph;


    if(rad<0)rad = 10;

    int size = size_slider.h;
    if ( leaf )
    {
        deph = width_slider.h;
        deph -=(width_slider.h-12)%10;
        deph/=10;

        if(leaf->animate == true)
        {
            y0=leaf->step*map.scale;
            float upper = (leaf->step)/sin(PI/2-rad*PI/180);
            x0 = upper * sin(rad*PI/180) *map.scale;
            SDL_SetRenderDrawColor( gRenderer, 255, 0, 0, 0xFF );
            animate(leaf);
        }
        else
        {
            y0=size*map.scale;
            float upper = size/sin(PI/2-rad*PI/180);
            x0 = upper * sin(rad*PI/180) *map.scale;
            SDL_SetRenderDrawColor( gRenderer, 255, 0, 0, 0xFF );
        }
        if(dir == 1)
        {
            SDL_RenderDrawLine(gRenderer,x,y,x+x0,y+y0);
            x+=x0;
            y+=y0;

        }
        else if(dir == -1)
        {
            SDL_RenderDrawLine(gRenderer,x,y,x-x0,y+y0);
            x-=x0;
            y+=y0;
        }

        node_tree.render(x-(node_tree.getWidth()*map.scale/5)/2, y - (node_tree.getHeight()*map.scale/5)/2,map.scale/5);
        leaf->Tvalue.render(x-(leaf->Tvalue.getHeight()*map.scale/8)/2, y-(leaf->Tvalue.getWidth()*map.scale/8)/2,map.scale/8);
        rad =rad - deph;
        Preorder(leaf->left, -1,x,y,rad);
        Preorder(leaf->right, 1,x,y,rad);
    }
}





node* btree::FindMin(node* root)
{
    while(root->left != NULL) root = root->left;
    return root;
}

node* btree::Delete(node *root, int key)
{
    if(root == NULL) return root;
    else if(key < root->key_value) root->left = Delete(root->left,key);
    else if(key > root->key_value) root->right = Delete(root->right, key);
    else
    {
        // Case 1: No Child
        if(root->left == NULL && root->right == NULL)
        {
            delete root;
            root = NULL;
            // Case 2: one child
        }
        else if(root->left == NULL)
        {
            node *temp = root;
            root = root->right;
            delete temp;
        }
        else if(root->right == NULL)
        {
            node *temp = root;
            root = root->left;
            delete temp;
        }
        else
        {
            node *temp = FindMin(root->right);
            root->key_value = temp->key_value;
            root->right = Delete(root->right, temp->key_value);
        }
    }
    return root;
}








btree tree;
int menuSelected;
int treeSelected;
void buttonManage(Lbutton *button, int total, int selected)
{
    for(int  i = 0; i < total; i++ )
    {
        if(button[i].clicked && selected != i+1)
        {
            button[i].clicked = false;
        }
    }
}




int *SCREEN_W = NULL;
int *SCREEN_H = NULL;

int main( int argc, char* args[] )
{

    int k = 0;
    map.temp = 1.5;
    map.scale = 1;
    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Load media
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
            bool quit = false;
            //Main loop flag
            //__________________________
            //*
            srand ( 5);
            for(int  i=10; i>=0; i--)
            {
                tree.insert(rand()%10);
                //tree.insert(i);
            }
            //_________________________*/
            //Event handler
            SDL_Event e;

            //While application is running

            //*SCREEN_W = SCREEN_WIDTH;

            //*SCREEN_H = SCREEN_HEIGHT;
            //std::cout << SCREEN_W << "   " << *SCREEN_W;


            SDL_Rect viewport_0; // pagrindinis langas
            SDL_Rect viewport_1; // menu langas
            SDL_Rect viewport_2; // draw langas
            SDL_Rect viewport_3; // input langas
            SDL_Rect windowFill;

            map.x =0;
            map.y =0;
            bool mapMove = false;
            bool resize = false;
            int Width;
            int Height;

            std::string inputText = "";
            gInputTextTexture.loadFromRenderedText( inputText.c_str(),gFontB, textColor );

            //Enable text input
            SDL_StartTextInput();
            SDL_ShowCursor(0);
            SDL_Point hef;
            hef.x = SCREEN_WIDTH*0.3;
            hef.y = SCREEN_HEIGHT*0.9;
            while( !quit )
            {
                SDL_GetWindowSize(gWindow,&Width,&Height);
                bool renderText = false;



                buttonUpdate(hef.x,Height*0.05,true,TOTAL_MENU_BUTTONS,buttonMenu);

                buttonUpdate(Width*0.1-12,Height*0.03,false,TOTAL_TREE_BUTTONS,buttonTree);




                viewport_0.x = 0;
                viewport_0.y = 0;
                viewport_0.w = Width;
                viewport_0.h = Height;

                viewport_1.x = 2;
                viewport_1.y = 2;
                viewport_1.w = hef.x;
                viewport_1.h = viewport_0.h-4;

                viewport_2.x = viewport_1.w+6;
                viewport_2.y = 2;
                viewport_2.w = Width-viewport_1.w-6;
                viewport_2.h = hef.y ;

                viewport_3.x = viewport_1.w+6;
                viewport_3.y = viewport_2.h +4;
                viewport_3.w = Width-viewport_1.w-4;
                viewport_3.h = Height-viewport_2.h-6;

                //Handle events on queue
                int x, y;
                SDL_GetMouseState( &x, &y );

                buttonManage(buttonMenu,TOTAL_MENU_BUTTONS,menuSelected);
                buttonManage(buttonTree,TOTAL_TREE_BUTTONS,treeSelected);
                while( SDL_PollEvent( &e ) != 0 )
                {
                    //User requests quit
                    if( e.type == SDL_QUIT || menuSelected == 4 )
                    {
                        quit = true;
                    }
                    else
                    {
                        if(x>=viewport_1.w-5 && x<= viewport_2.x)
                        {


                            if(e.type == SDL_MOUSEBUTTONUP)resize=false;
                            if(e.type == SDL_MOUSEBUTTONDOWN)
                            {
                                resize = true;
                            }
                            if(resize && e.type == SDL_MOUSEMOTION)
                            {
                                if(x>=0 && x<=10  && resize)
                                {
                                    resize = false;
                                    hef.x =180;
                                }
                                else if(x<90 && resize)
                                {
                                    resize = false;
                                    hef.x = 0;
                                }
                                else
                                    hef.x = x;
                            }
                        }
                        else

                            if(x>=viewport_3.x && y>=viewport_2.h-5 && y<=viewport_3.y)
                            {
                                if(e.type == SDL_MOUSEBUTTONUP)resize=false;
                                if(e.type == SDL_MOUSEBUTTONDOWN)
                                {
                                    resize = true;
                                }
                                if(resize && e.type == SDL_MOUSEMOTION)
                                {
                                    if(y<=Height && y>=Height-10  && resize)
                                    {
                                        resize = false;
                                        hef.y =Height*0.9;
                                    }
                                    else
                                        //*
                                        if(y>Height-20 && resize)
                                        {
                                            resize = false;
                                            hef.y = Height-5;
                                        }
                                        else
                                            //*/
                                            hef.y = y;
                                }
                            }
                            else

                                //*


                                if(x>=size_slider.x-5 && x<=size_slider.w+5  && y>=size_slider.y-10+viewport_1.y && y<=size_slider.y+10+viewport_1.y)
                                {
                                    if(e.type == SDL_MOUSEBUTTONUP)resize=false;
                                    if(e.type == SDL_MOUSEBUTTONDOWN)
                                    {
                                        resize = true;
                                    }

                                    if(resize || e.type == SDL_MOUSEMOTION)
                                    {

                                        if(x>=12 && x<= 168 && resize)
                                        {

                                            size_slider.h  = x;
                                        }
                                        else if(x>168 && resize)
                                        {
                                            size_slider.h = 168;
                                        }
                                        else if(x<12 && resize)
                                        {
                                            size_slider.h = 12;
                                        }
                                    }




                                }

                                else if(x>=width_slider.x-5 && x<=width_slider.w+5  && y>=width_slider.y-10+viewport_1.y && y<=width_slider.y+10+viewport_1.y)
                                {
                                    if(e.type == SDL_MOUSEBUTTONUP)resize=false;
                                    if(e.type == SDL_MOUSEBUTTONDOWN)
                                    {
                                        resize = true;
                                    }

                                    if(resize || e.type == SDL_MOUSEMOTION)
                                    {

                                        if(x>=12 && x<= 168 && resize)
                                        {

                                            width_slider.h  = x;
                                        }
                                        else if(x>168 && resize)
                                        {
                                            width_slider.h = 168;
                                        }
                                        else if(x<12 && resize)
                                        {
                                            width_slider.h = 12;
                                        }
                                    }




                                }
                                else if(x>=angle_slider.x-5 && x<=angle_slider.w+5  && y>=angle_slider.y-10+viewport_1.y && y<=angle_slider.y+10+viewport_1.y)
                                {
                                    if(e.type == SDL_MOUSEBUTTONUP)resize=false;
                                    if(e.type == SDL_MOUSEBUTTONDOWN)
                                    {
                                        resize = true;
                                    }

                                    if(resize || e.type == SDL_MOUSEMOTION)
                                    {

                                        if(x>=angle_slider.x && x<= angle_slider.w && resize)
                                        {

                                            angle_slider.h  = x;
                                        }
                                        else if(x>101 && resize)
                                        {
                                            angle_slider.h = 101;
                                        }
                                        else if(x<30 && resize)
                                        {
                                            angle_slider.h = 30;
                                        }
                                    }




                                }
                                else resize = false;




                        //*/


                        //____________________________

                        if( e.type == SDL_KEYDOWN )
                        {
                            //Handle backspace
                            if( e.key.keysym.sym == SDLK_BACKSPACE && inputText.length() > 0 )
                            {
                                //lop off character
                                inputText.pop_back();
                                renderText = true;
                            }
                            //Handle copy
                            else if( e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL )
                            {
                                SDL_SetClipboardText( inputText.c_str() );
                            }
                            //Handle paste
                            else if( e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL )
                            {
                                inputText = SDL_GetClipboardText();
                                renderText = true;
                            }
                            //enter handle
                            // reikia kad rasyti tik paspaudus ant rasymo lauko
                            else if(e.key.keysym.sym == 13)
                            {
                                switch(treeSelected)
                                {
                                case 1:
                                {
                                    int test;
                                    std::stringstream convert(inputText);
                                    convert>>test;
                                    tree.insert(test);
                                    inputText = "";
                                    break;
                                }

                                case 2:
                                {
                                    int test;
                                    std::stringstream convert(inputText);
                                    convert>>test;
                                    //tree.search(test);
                                    inputText = "";
                                    break;
                                }
                                case 3:
                                {
                                    int test;
                                    std::stringstream convert(inputText);
                                    convert>>test;
                                    tree.Delete(test);
                                    inputText = "";
                                    break;
                                }
                                }

                            }

                        }
                        //Special text input event
                        else if( e.type == SDL_TEXTINPUT )
                        {
                            //Not copy or pasting
                            if( !( ( e.text.text[ 0 ] == 'c' || e.text.text[ 0 ] == 'C' ) && ( e.text.text[ 0 ] == 'v' || e.text.text[ 0 ] == 'V' ) && SDL_GetModState() & KMOD_CTRL )&& inputText.length()<14 )
                            {
                                //Append character

                                inputText += e.text.text;
                                renderText = true;
                            }
                        }

                        //____________________________



                        //if(viewport_1.w>x && x>viewport_1.x &&  viewport_1.h>y && y>viewport_1.y)
                        for(int  i = 0; i < TOTAL_MENU_BUTTONS; ++i )
                        {
                            buttonMenu[ i ].handleEvent( &e,  viewport_1, i, &menuSelected);
                        }
                        //else
                        //*
                        //if(x>(viewport_0.w + 4)*0.3 && y>viewport_0.h*0.9 && x<viewport_3.w+(viewport_0.w + 4)*0.3 && y<viewport_3.h+viewport_0.h*0.9){
                        SDL_RenderSetViewport( gRenderer, &viewport_3 );//_________
                        for(int  i = 0; i < TOTAL_TREE_BUTTONS; ++i )
                        {
                            buttonTree[ i ].handleEvent( &e, viewport_3, i, &treeSelected);
                        }
                        //}
                        //*/
                        if(x>viewport_2.x && y>viewport_2.y && x<viewport_2.x+viewport_2.w && y<viewport_2.h)
                        {

                            if(e.type == SDL_MOUSEWHEEL)
                            {
                                if(e.wheel.y<0 && map.scale>0.2)
                                {
                                    map.scale-=0.1;
                                }
                                if(e.wheel.y>0)
                                {
                                    map.scale+=0.1;
                                }
                                if(map.scale<0.2)map.scale=0.2;
                            }
                            if(e.type == SDL_MOUSEBUTTONUP)mapMove=false;
                            if(e.type == SDL_MOUSEBUTTONDOWN)
                            {
                                mapMove = true;
                                mouseMove.x = x - viewport_2.x - map.x;
                                mouseMove.y = y - viewport_2.y - map.y;
                            }
                            if(mapMove)
                            {
                                map.x=x - mouseMove.x - viewport_2.x;
                                map.y=y - mouseMove.y - viewport_2.y;
                                //if(map.y<0)map.y=0;
                                //if(map.x<0)map.x=0;
                                //if(map.x>viewport_2.w)map.x=viewport_2.w;
                                //if(map.y>viewport_2.h)map.y=viewport_2.h;
                                mapMove=true;
                            }
                        }
                        else
                        {
                            mapMove=false;








                        }


                        if( e.type == SDL_KEYDOWN )
                        {
                            //Select surfaces based on key press

                            switch( e.key.keysym.sym )
                            {
                            case SDLK_UP:
                                break;
                            }


                        }
                    }
                }
                SDL_RenderSetViewport( gRenderer, &viewport_0 );
                windowFill = { 0, 0, viewport_0.w, viewport_0.h};
                SDL_SetRenderDrawColor( gRenderer, 240, 240, 240, 0xFF );
                SDL_RenderFillRect( gRenderer, &windowFill );


                //Update the surface
                SDL_RenderSetViewport( gRenderer, &viewport_1 );
                windowFill = { 0, 0, viewport_1.w, viewport_1.h };
                SDL_SetRenderDrawColor( gRenderer, 130, 135, 144, 0xFF );
                SDL_RenderDrawRect( gRenderer, &windowFill );
                for(int  i = 0; i < TOTAL_MENU_BUTTONS; ++i )
                {
                    buttonMenu[ i ].render(i,1);

                }
                switch(menuSelected)
                {
                case 2:
                {
                    windowFill = { 6, (buttonMenu[ 0 ].BUTTON_HEIGHT+4)*TOTAL_MENU_BUTTONS+windowOptionText[0].getHeight(), viewport_1.w-12, viewport_1.h-(buttonMenu[ 0 ].BUTTON_HEIGHT+4)*TOTAL_MENU_BUTTONS-8 };


                    SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0xFF );
                    SDL_RenderDrawRect( gRenderer, &windowFill );
                    //size_slider
                    size_slider.x = 10;
                    size_slider.y = (buttonMenu[ 0 ].BUTTON_HEIGHT+4)*TOTAL_MENU_BUTTONS+windowOptionText[0].getHeight() + 40;
                    size_slider.w = 168;
                    SDL_RenderDrawLine(gRenderer,size_slider.x,size_slider.y,size_slider.w,size_slider.y);
                    SDL_SetRenderDrawColor( gRenderer, 0, 255, 100, 0xFF );
                    windowFill = { size_slider.h-5,size_slider.y-10,10,20  };
                    SDL_RenderFillRect( gRenderer, &windowFill );
                    windowOptionText[1].render(size_slider.x,size_slider.y-20,1);
                    //angle_slider
                    SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0xFF );
                    angle_slider.x = 30;
                    angle_slider.y = size_slider.y + 40;
                    angle_slider.w = 101;
                    SDL_RenderDrawLine(gRenderer,angle_slider.x,angle_slider.y,angle_slider.w,angle_slider.y);
                    SDL_SetRenderDrawColor( gRenderer, 0, 255, 100, 0xFF );
                    windowFill = { angle_slider.h-5,angle_slider.y-10,10,20  };
                    SDL_RenderFillRect( gRenderer, &windowFill );
                    windowOptionText[2].render(angle_slider.x,angle_slider.y-20,1);
                    //width_slider
                    SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0xFF );
                    width_slider.x = 10;
                    width_slider.y = angle_slider.y + 40;
                    width_slider.w = 168;
                    SDL_RenderDrawLine(gRenderer,width_slider.x,width_slider.y,width_slider.w,width_slider.y);
                    SDL_SetRenderDrawColor( gRenderer, 0, 255, 100, 0xFF );
                    windowFill = { width_slider.h-5,width_slider.y-10,10,20  };
                    SDL_RenderFillRect( gRenderer, &windowFill );
                    windowOptionText[3].render(width_slider.x,width_slider.y-20,1);
                    // rest otion box
                    SDL_SetRenderDrawColor( gRenderer, 240, 240, 240, 0xFF );
                    windowFill = { 16, (buttonMenu[ 0 ].BUTTON_HEIGHT+4)*TOTAL_MENU_BUTTONS+windowOptionText[0].getHeight()/2,windowOptionText[0].getWidth(),windowOptionText[0].getHeight()  };
                    SDL_RenderFillRect( gRenderer, &windowFill );
                    windowOptionText[0].render(16, (buttonMenu[ 0 ].BUTTON_HEIGHT+4)*TOTAL_MENU_BUTTONS+windowOptionText[0].getHeight()/2,1);
                    break;
                }
                case 3:
                {
                    windowFill = { 6, (buttonMenu[ 0 ].BUTTON_HEIGHT+4)*TOTAL_MENU_BUTTONS+windowCreditText[0].getHeight(), viewport_1.w-12, viewport_1.h-(buttonMenu[ 0 ].BUTTON_HEIGHT+4)*TOTAL_MENU_BUTTONS-8 };


                    SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0xFF );
                    SDL_RenderDrawRect( gRenderer, &windowFill );
                    // rest otion box
                    SDL_SetRenderDrawColor( gRenderer, 240, 240, 240, 0xFF );
                    windowFill = { 16, (buttonMenu[ 0 ].BUTTON_HEIGHT+4)*TOTAL_MENU_BUTTONS+windowCreditText[0].getHeight()/2,windowCreditText[0].getWidth(),windowCreditText[0].getHeight()  };
                    SDL_RenderFillRect( gRenderer, &windowFill );
                    windowCreditText[0].render(16, (buttonMenu[ 0 ].BUTTON_HEIGHT+4)*TOTAL_MENU_BUTTONS+windowCreditText[0].getHeight()/2,1);
                    break;
                }

                }





                SDL_RenderSetViewport( gRenderer, &viewport_2 );
                windowFill = { 0, 0, viewport_2.w, viewport_2.h };
                SDL_SetRenderDrawColor( gRenderer, 83, 83, 83, 0xFF );
                SDL_RenderFillRect( gRenderer, &windowFill );
                SDL_SetRenderDrawColor( gRenderer, 130, 135, 144, 0xFF );
                SDL_RenderDrawRect( gRenderer, &windowFill );




                for(int i =0; i<viewport_2.w; i++)
                {

                    int x0 = i*10*map.scale;
                    int x2 = i*10*map.scale;
                    if(i%10 == 0)
                        SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0xFF );
                    else
                        SDL_SetRenderDrawColor( gRenderer, 55, 55, 55, 0xFF );
                    SDL_RenderDrawLine(gRenderer,map.x+x0,0,map.x+x0,viewport_2.h);
                    SDL_RenderDrawLine(gRenderer,map.x-x2,0,map.x-x2,viewport_2.h);
                    //if(x0>=viewport_2.w)i = viewport_2.w;

                }
                for(int  i = 0; i<viewport_2.h; i++)
                {
                    int y0 = i*10*map.scale;
                    int y2 = i*10*map.scale;
                    if(i%10 == 0)
                        SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0xFF );
                    else
                        SDL_SetRenderDrawColor( gRenderer, 55, 55, 55, 0xFF );

                    SDL_RenderDrawLine(gRenderer,0,map.y+y0,viewport_2.w,map.y+y0);
                    SDL_RenderDrawLine(gRenderer,0,map.y-y2,viewport_2.w,map.y-y2);
                    //if(y0>=viewport_2.h)k = viewport_2.h;
                }
                SDL_SetRenderDrawColor( gRenderer, 130, 135, 144, 0xFF );
                tree.PreorderRender();
                SDL_RenderSetViewport( gRenderer, &viewport_3 );
                windowFill = { 0, 0, viewport_3.w, viewport_3.h };
                SDL_SetRenderDrawColor( gRenderer, 130, 135, 144, 0xFF );
                SDL_RenderDrawRect( gRenderer, &windowFill );
                for(int  i = 0; i < TOTAL_TREE_BUTTONS; ++i )
                {
                    buttonTree[ i ].render(i,3);

                }

//____________________________
                if( renderText )
                {
                    //Text is not empty
                    if( inputText != "" )
                    {
                        //Render new text
                        gInputTextTexture.loadFromRenderedText( inputText.c_str(),gFontB, textColor );
                    }
                    //Text is empty
                    else
                    {
                        //Render space texture
                        gInputTextTexture.loadFromRenderedText( " ",gFont, textColor );
                    }
                }

                gPromptTextTexture.render( 2, viewport_3.h - gPromptTextTexture.getHeight()-2,1 );
                SDL_Rect inputBox = {gPromptTextTexture.getWidth()+2, viewport_3.h - gPromptTextTexture.getHeight()-2,100,gPromptTextTexture.getHeight()};

                SDL_SetRenderDrawColor( gRenderer, 255, 255, 255, 0xFF );
                SDL_RenderFillRect( gRenderer, &inputBox );
                SDL_SetRenderDrawColor( gRenderer, 130, 135, 144, 0xFF );
                SDL_RenderDrawRect( gRenderer, &inputBox );
                gInputTextTexture.render( gPromptTextTexture.getWidth()+4, viewport_3.h - gPromptTextTexture.getHeight()-2,1);
//_________________________________-
                SDL_RenderSetViewport( gRenderer, &viewport_0 );
                if(x>=viewport_1.w-5 && x<= viewport_2.x)
                    cursor.render(x-5,y,1,&cursor_size_x);
                else if(x>=viewport_3.x && y>=viewport_2.h-5 && y<=viewport_3.y)
                    cursor.render(x,y-5,1,&cursor_size_y);
                else if(x>=size_slider.h-5 && x<=size_slider.h+5  && y>=size_slider.y-10+viewport_1.y && y<=size_slider.y+10+viewport_1.y)
                    cursor.render(x,y,1,&cursor_pan_select);
                else if(x>=angle_slider.h-5 && x<=angle_slider.h+5  && y>=angle_slider.y-10+viewport_1.y && y<=angle_slider.y+10+viewport_1.y)
                    cursor.render(x,y,1,&cursor_pan_select);
                else if(x>=width_slider.h-5 && x<=width_slider.h+5  && y>=width_slider.y-10+viewport_1.y && y<=width_slider.y+10+viewport_1.y)
                    cursor.render(x,y,1,&cursor_pan_select);
                else if(x>viewport_2.x && y>viewport_2.y && x<viewport_2.x+viewport_2.w && y<viewport_2.h && !mapMove)
                    cursor.render(x,y,1,&cursor_pan);
                else if(x>viewport_2.x && y>viewport_2.y && x<viewport_2.x+viewport_2.w && y<viewport_2.h && mapMove)
                    cursor.render(x,y,1,&cursor_pan_select);
                else
                    cursor.render(x,y,1,&cursor_plain);

                SDL_RenderPresent( gRenderer );

            }
            SDL_StopTextInput();
        }
    }

    //Free resources and close SDL
    close();

    return 0;
}
