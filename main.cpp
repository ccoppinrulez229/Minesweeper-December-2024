#include <iostream>
#include <SFML/Graphics.hpp>
#include <vector>
#include <fstream>
#include <cctype>
#include <random>
#include <string>
#include <chrono>
#include <thread>
using namespace std;
using namespace std::chrono;

#include <sstream>

void setTextPosition(sf::Text &text, float x, float y) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    text.setPosition(sf::Vector2f(x, y));
}

void setCursorPosition(sf::RectangleShape &rectangle, float x, float y,int user_name_len) {
    sf::FloatRect textRect = rectangle.getLocalBounds();
    rectangle.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
    if (user_name_len<=4) {
        rectangle.setPosition(sf::Vector2f(x+(user_name_len*8), y));
    }
    else if (user_name_len<=7) {
        rectangle.setPosition(sf::Vector2f(x+(user_name_len*8)-7, y));
    }
    else if (user_name_len<=9) {
        rectangle.setPosition(sf::Vector2f(x+(user_name_len*8)-10, y));
    }
    else {
        rectangle.setPosition(sf::Vector2f(x+(user_name_len*8)-15, y));
    }

}

enum TileState {
    Up,
    Down
};

class Tile {
    vector<Tile*> adjacent_tiles; //can have UP TO 8 neighbors
    const sf::Texture* up_texture;
    const sf::Texture* down_texture;
    sf::Sprite sprite;
    TileState state;
    bool mine = false;
    int x_pos;
    int y_pos;
    bool flag = false;
    bool is_revealed = false;
    int num_of_adjacent_mines = -1;
    bool game_over = false;
    bool game_paused = false;

public:
    Tile(const sf::Texture& up_texture, const sf::Texture& down_texture, int x, int y) {
        this->up_texture = &up_texture;
        this->down_texture = &down_texture;
        this->sprite = sf::Sprite(up_texture);
        this->state = Up;
        x_pos = x;
        y_pos = y;
        sprite.setPosition(x, y);
    }
    sf::Sprite& get_sprite() {
        return this->sprite;
    }

    void click() {
        if (state==Up) {
            reveal_tile();
        }
    }

    void reveal_tile() {
        if (!mine && !flag) { //checks if the tile being revealed is not a mine or flagged
            is_revealed = true;
            num_of_adjacent_mines = 0;
            for (int i=0 ; i<adjacent_tiles.size() ; i++) {
                if (adjacent_tiles[i]->CheckIfMine()==true) {
                    num_of_adjacent_mines+=1;
                }
            }
            if (num_of_adjacent_mines==0) {
                for (int i=0 ; i<adjacent_tiles.size() ; i++) {
                    if (adjacent_tiles[i]->IsRevealed()==false && adjacent_tiles[i]->IsFlagged()==false && adjacent_tiles[i]->CheckIfMine()==false) {
                        adjacent_tiles[i]->reveal_tile();
                    }
                }
            }
            state = Down;
        }
        else if (mine && !flag) {
            game_over = true;
        }
    }

    void rightclick() {
        if (flag == false && state == Up) {
            flag = true;
        }
        else if (flag == true && state == Up) {
            flag = false;
        }
    }
    bool IsFlagged() {
        return flag;
    }

    bool IsRevealed() {
        return is_revealed;
    }

    void SetAsMine() {
        mine = true;
    }

    bool CheckIfMine() {
        return mine;
    }

    bool CheckForGameOver() {
        return game_over;
    }

    int GetXPosition() {
        return x_pos;
    }

    int GetYPosition() {
        return y_pos;
    }

    void PushToAdjacentVector(Tile* tile) {
        adjacent_tiles.push_back(tile);
        if (adjacent_tiles.size()>8) {
            cout << "max size exceeded!" << endl;
        }
    }

    int get_adjacent_mines() {
        return num_of_adjacent_mines;
    }

    void set_texture_to_down() {
        sprite.setTexture(*down_texture);
    }

    bool CheckIfRevealed() {
        return is_revealed;
    }

    void set_texture_to_up() {
        sprite.setTexture(*up_texture);
    }
};

void CreateBoard(vector<Tile>& tiles,int height, int width, int maxcount,sf::Texture& up,sf::Texture& down) {
    int current_tile_count=0;
    for (int y = 0; y < height; y += 32) {
        for (int x = 0; x < width; x += 32) { //Creates one row of tiles
            if (current_tile_count < maxcount) { //will stop once the amount of tiles is reached
                Tile tile = Tile(up,down,x,y);
                tiles.push_back(tile);
                current_tile_count+=1;
            }
        }
    }
}

void SetAdjacentTiles (vector<Tile>& tiles) { //will be called AFTER the tile vector is made
    for (int i = 0; i < tiles.size(); i++) {

        int current_x_pos = tiles[i].GetXPosition() - 32;
        int current_y_pos = tiles[i].GetYPosition() - 32;
        int hard_x_pos = tiles[i].GetXPosition();
        int hard_y_pos = tiles[i].GetYPosition();

        // Iterate over a 3x3 grid of adjacent tiles around the current tile
        for (int col = 0; col < 3; col++) {
            for (int row = 0; row < 3; row++) {

                // will check whether or not the current tile needs to be skipped
                if (current_x_pos == hard_x_pos && current_y_pos == hard_y_pos) {
                    current_x_pos += 32;
                    continue;
                }

                // Check if there's a tile at the current position
                for (int j = 0; j < tiles.size(); j++) {
                    if (tiles[j].GetXPosition() == current_x_pos && tiles[j].GetYPosition() == current_y_pos) {
                        // If found, add it to the adjacent tiles list
                        tiles[i].PushToAdjacentVector(&tiles[j]);
                        //cout << "Found adjacent tile for tile " << i << " at position (" << current_x_pos << "," << current_y_pos << ")" << endl;
                    }
                }

                current_x_pos += 32;
            }

            current_y_pos += 32; //moves one row down to check
            current_x_pos = tiles[i].GetXPosition() - 32; //resets the x position to check the next row
        }
    }
}

class Mine_Sprite {
    const sf::Texture* mine_texture;
    sf::Sprite sprite;

public:
    Mine_Sprite(const sf::Texture& happy_face, int x, int y) {
        this->mine_texture = &happy_face;
        this->sprite = sf::Sprite(*mine_texture);
        sprite.setPosition(x,y);
    }

    sf::Sprite& get_sprite() {
        return this->sprite;
    }
};

void GenerateMines(vector<Tile>& tiles,int mine_count,vector<Mine_Sprite>& mines, sf::Texture& mine_sprite) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(1,50);
    int current_mine_count = 0;
    while (current_mine_count<mine_count) { //number of mines being generated is currently a little off by a few mines. NEED TO FIX
        for (int i=0 ; i<tiles.size() ; i++) {
            int random_number = dis(gen);
            if (random_number == 10 && current_mine_count < mine_count && tiles[i].CheckIfMine()==false) {
                tiles[i].SetAsMine();
                Mine_Sprite mine = Mine_Sprite(mine_sprite,tiles[i].GetXPosition(),tiles[i].GetYPosition());
                mines.push_back(mine);
                current_mine_count+=1;
                //cout << "mine" << current_mine_count << "placed at" << "(" << tiles[i].GetXPosition() << "," << tiles[i].GetYPosition() << ")" << endl;
            }
            else {
                continue;
            }
        }
    }
}

class Happy_Face_Button {
    const sf::Texture* happy_face_texture;
    const sf::Texture* lose_face_texture;
    const sf::Texture* win_face_texture;
    sf::Sprite sprite;
    int x_pos;
    int y_pos;

public:
    Happy_Face_Button(const sf::Texture& happy_face, int x, int y,const sf::Texture& lose_face, const sf::Texture& win_face) {
        this->happy_face_texture = &happy_face;
        this->lose_face_texture = &lose_face;
        this->win_face_texture = &win_face;
        this->sprite = sf::Sprite(*happy_face_texture);
        sprite.setPosition(x,y);
        x_pos = x;
        y_pos = y;
    }

    sf::Sprite& get_sprite() {
        return this->sprite;
    }

    void change_texture(string mood) {
        if (mood == "happy") {
            sprite.setTexture(*happy_face_texture);
        }

        else if (mood == "lose") {
            sprite.setTexture(*lose_face_texture);
        }

        else if (mood == "win") {
            sprite.setTexture(*win_face_texture);
        }
    }
};

class Debug_Button {
    const sf::Texture* debug_button_texture;
    sf::Sprite sprite;

public:
    Debug_Button(const sf::Texture& debug_button, int x, int y) {
        this->debug_button_texture = &debug_button;
        this->sprite = sf::Sprite(*debug_button_texture);
        sprite.setPosition(x,y);
    }

    sf::Sprite& get_sprite() {
        return this->sprite;
    }
};

class Play_Pause_Button {
    // Store pointers to texture to avoid storing a copy (it can get very expensive very quickly!)
    const sf::Texture* play_texture;
    const sf::Texture* pause_texture;
    sf::Sprite sprite;
    TileState state;
    bool game_paused = false;
    bool previous_pause_state = false;

public:
    // Pass textures by reference and store by pointer to avoid copying the textures.
    // Pass x and y to indicate where on the screen the tile should appear
    Play_Pause_Button(const sf::Texture& play_texture, const sf::Texture& pause_texture, int x, int y) {
        // Set the texture pointers
        this->play_texture = &play_texture;
        this->pause_texture = &pause_texture;
        // Set sprite to 'up' state and texture
        this->sprite = sf::Sprite(pause_texture);
        this->state = Up;
        // Set the sprite's position on the screen
        sprite.setPosition(x, y);
    }
    // Getter for the sprite
    sf::Sprite& get_sprite() {
        return this->sprite;
    }
    // Function which outlines the logic of a tile being clicked
    void click() {
        // Check the current state and flip to the opposite state
        if (state == Up) {
            sprite.setTexture(*pause_texture);
            state = Down;
            game_paused = false;
        }
        else if (state == Down) { // State is down
            sprite.setTexture(*play_texture);
            state = Up;
            game_paused = true;
        }
    }
    bool IsGamePaused() {
        return game_paused;
    }
    void SwitchPreviousPauseState() {
        previous_pause_state = !previous_pause_state;
    }
    bool GetPreviousPauseState() {
        return previous_pause_state;
    }
};

class Leaderboard_Button {
    const sf::Texture* leaderboard_button_texture;
    sf::Sprite sprite;

public:
    Leaderboard_Button(const sf::Texture& happy_face, int x, int y) {
        this->leaderboard_button_texture = &happy_face;
        this->sprite = sf::Sprite(*leaderboard_button_texture);
        sprite.setPosition(x,y);
    }

    sf::Sprite& get_sprite() {
        return this->sprite;
    }
};

void DrawCounter(int desired_digit_count, int& current_count, sf::Sprite& digit_sprite, sf::RenderWindow& window,int rows, int cols) {
    string countstring = to_string(abs(current_count));
    string zeroes;
    if (countstring.length() != desired_digit_count) { //checks if the current number of mines needs extra 0s behind it
        int zeros_to_add = desired_digit_count - countstring.length();
        for (int i=0 ; i<zeros_to_add ; i++) {
            zeroes += "0";
        }
        countstring = zeroes + countstring;
    }

    if (current_count<0) { //checks if the current mine count is negative. if so, draw a negative symbol
        digit_sprite.setPosition(12,32*(rows+0.5)+16);
        digit_sprite.setTextureRect(sf::IntRect(21*10,0,21,32));
        window.draw(digit_sprite);
    }

    int x_pos = 33;
    for (int i=0 ; i<countstring.length() ; i++) { //looks at each digit of the current count and draw them at specific positions
        digit_sprite.setPosition(x_pos,32*(rows+0.5)+16);
        int digit = stoi(std::string(1,countstring[i]));
        digit_sprite.setTextureRect(sf::IntRect(21*digit,0,21,32));
        window.draw(digit_sprite);
        x_pos += 21;
    }
}

void DrawTimer(int minutes, int seconds, sf::Sprite& digit_sprite, sf::RenderWindow& window, int row, int col) {
    string minstring = to_string(minutes);
    string secstring = to_string(seconds);

    if (minstring.length() != 2) { //checks if the current number of mines needs extra 0s behind it
        minstring = "0" + minstring;
    }
    if (secstring.length() != 2) {
        secstring = "0" + secstring;
    }

    int x_pos = (col*32)-97;
    for (int i=0 ; i<minstring.length() ; i++) { //looks at each digit of the current count and draw them at specific positions
        digit_sprite.setPosition(x_pos,32*(row+0.5)+16);
        int digit = stoi(std::string(1,minstring[i]));
        digit_sprite.setTextureRect(sf::IntRect(21*digit,0,21,32));
        window.draw(digit_sprite);
        x_pos += 21;
    }

    x_pos = (col*32)-54;
    for (int i=0 ; i<secstring.length() ; i++) { //looks at each digit of the current count and draw them at specific positions
        digit_sprite.setPosition(x_pos,32*(row+0.5)+16);
        int digit = stoi(std::string(1,secstring[i]));
        digit_sprite.setTextureRect(sf::IntRect(21*digit,0,21,32));
        window.draw(digit_sprite);
        x_pos += 21;
    }
}

bool CheckIfWin(vector<Tile>& tiles,int numoftilestoreveal, int numofmines) {
    int revealed_tiles=0;
    int flagged_mines=0;
    for (int i=0 ; i<tiles.size() ; i++) {
        if (tiles[i].IsRevealed()) {
            revealed_tiles += 1;
        }
        if (tiles[i].CheckIfMine() && tiles[i].IsFlagged()) {
            flagged_mines +=1;
        }
    }

    if ((revealed_tiles == numoftilestoreveal)) {
        return true;
    }

    return false;
}

class Timer {
    sf::Clock clock;
    sf::Time start_time;
    sf::Time elapsed_time;
    bool is_paused;

public:
    Timer() {
        elapsed_time = sf::Time::Zero;
        is_paused = true;
    }
    void StartTimer() {
        if (is_paused) {
            start_time = clock.getElapsedTime() - elapsed_time;
            is_paused = false;
        }
    }
    void PauseTimer() {
        if (!is_paused) {
            elapsed_time = clock.getElapsedTime() - start_time;
            is_paused = true;
        }
    }
    void ResetTimer() {
        elapsed_time = sf::Time::Zero;
        if (!is_paused) {
            start_time = clock.getElapsedTime();
        }
    }
    sf::Time getElapsedTime() {
        if (is_paused) {
            return elapsed_time;
        }
        return clock.getElapsedTime() - start_time;
    }
    int getMinutes() {
        return static_cast<int>(getElapsedTime().asSeconds()) / 60;
    }
    int getSeconds() {
        return static_cast<int>(getElapsedTime().asSeconds()) % 60;
    }
};

void LeaderBoard_Window(sf::Font& font, int window_width, int window_height, Play_Pause_Button& button, bool& leaderboard_opened,bool& win,string playername,int minutes,int seconds, bool& leaderboard_updated, bool& pause,Timer& timer, bool& hide_contents) {
    sf::Text leaderboard_header;
    leaderboard_header.setFont(font);
    leaderboard_header.setString("LEADERBOARD");
    leaderboard_header.setCharacterSize(20);
    leaderboard_header.setFillColor(sf::Color::White);
    leaderboard_header.setStyle(sf::Text::Bold | sf::Text::Underlined);
    setTextPosition(leaderboard_header,window_width/2,(window_height/2)-120);

    sf::Text leaderboard_contents;
    leaderboard_contents.setFont(font);
    leaderboard_contents.setCharacterSize(18);
    leaderboard_contents.setFillColor(sf::Color::White);
    leaderboard_contents.setStyle(sf::Text::Bold);

    sf::RenderWindow leaderboard(sf::VideoMode(window_width, window_height), "Minesweeper",sf::Style::Close);
    fstream PlayerScores("files/leaderboard.txt");
    string line;
    string time;
    string name;
    vector<vector<string>> players;
    for (int i=0 ; i<5 ; i++) {
        getline(PlayerScores,line);
        stringstream ss(line);
        string token;
        vector<string> tempvec;
        while (getline(ss,token,',')) {
            tempvec.push_back(token);
        }
        players.push_back(tempvec);
    }
    if (win == true) {
        //cout << "Completed in" << minutes << "minutes and" << seconds << "seconds!" << endl;
        string min_string = to_string(minutes);
        if (min_string.length()==1) {
            min_string = "0" + min_string;
        }
        string sec_string = to_string(seconds);
        if (sec_string.length()==1) {
            sec_string = "0" + sec_string;
        }

        vector<string> player_result = {min_string + ":" + sec_string , " " + playername + "*"};
        for (int i=0 ; i<players.size() ; i++) {
            string player_time = players[i][0];
            int player_min = stoi(player_time.substr(0,2));
            int player_sec = stoi(player_time.substr(player_time.length()-2));

            if ((minutes < player_min) && (minutes !=player_min) && leaderboard_updated == false) {
                players.insert(players.begin()+i,player_result);
                leaderboard_updated = true;
                break;
            }
            else if ((minutes == player_min) && (seconds < player_sec) && (seconds != player_sec) && leaderboard_updated == false) {
                players.insert(players.begin()+i,player_result);
                leaderboard_updated = true;
                break;
            }
        }
    }
    /*for (int i=0 ; i<players.size() ; i++) {
        cout << players[i][0] << ": " << players[i][1] << endl;
    }*/
    PlayerScores.close();

    ofstream WritePlayerScores("files/leaderboard.txt");

    string ranking = "";
    for (int i=0 ; i<5 ; i++) {
        ranking += to_string(i+1) + "\t" + players[i][0] + "\t" + players[i][1] + "\n\n";
        if (players[i][1].find("*") != std::string::npos) {
            players[i][1].pop_back();
        }
        WritePlayerScores << players[i][0] + "," << players[i][1] << endl;
    }
    WritePlayerScores.close();
    leaderboard_contents.setString(ranking);
    setTextPosition(leaderboard_contents,window_width/2,(window_height/2)+20);


    while (leaderboard.isOpen()) {
        // Event loop
        sf::Event event;
        while (leaderboard.pollEvent(event)) {
            leaderboard.requestFocus();

            // Close event
            if (event.type == sf::Event::EventType::Closed) {
                if (button.IsGamePaused() && !button.GetPreviousPauseState()) {
                    button.click();
                }
                if (!button.GetPreviousPauseState()) {
                    pause = false;
                    timer.StartTimer();
                }
                leaderboard_opened = false;
                hide_contents = false;
                leaderboard.close();
                break;
            }


        }
        leaderboard.clear(sf::Color::Blue);
        leaderboard.draw(leaderboard_header);
        leaderboard.draw(leaderboard_contents);
        leaderboard.display();
    }
}

int main() {
    ifstream Config("files/config.cfg"); //loads config file

    //sets window width and height
    string line;
    getline(Config,line);
    int window_width=stoi(line)*32; //sets the window width based off the setting in the config.cfg file
    int leaderboard_width=stoi(line)*16;
    getline(Config,line);
    int window_height=(stoi(line)*32)+100; //sets the window height based off the setting in the config.cfg file
    int leaderboard_height = (stoi(line)*16)+50;

    //sets default font that will be used throughout the app
    sf::Font font;
    if (!font.loadFromFile("files/font.ttf")) {
        cout << "Font not found!" << endl;
        return 1;
    }

    //welcome screen

    sf::Text welcome_text;
    welcome_text.setFont(font);
    welcome_text.setString("WELCOME TO MINESWEEPER!");
    welcome_text.setCharacterSize(24);
    welcome_text.setFillColor(sf::Color::White);
    welcome_text.setStyle(sf::Text::Bold | sf::Text::Underlined);
    setTextPosition(welcome_text,window_width/2,(window_height/2)-150);

    sf::Text enter_name_text;
    enter_name_text.setFont(font);
    enter_name_text.setString("Enter your name:");
    enter_name_text.setCharacterSize(20);
    enter_name_text.setFillColor(sf::Color::White);
    enter_name_text.setStyle(sf::Text::Bold);
    setTextPosition(enter_name_text,window_width/2,(window_height/2)-75);

    sf::Text user_typed_name;
    user_typed_name.setFont(font);
    user_typed_name.setString("");
    user_typed_name.setCharacterSize(18);
    user_typed_name.setFillColor(sf::Color::Yellow);
    user_typed_name.setStyle(sf::Text::Bold);
    setTextPosition(user_typed_name,window_width/2,(window_height/2)-45);


    sf::RectangleShape cursor_indicator;
    cursor_indicator.setSize(sf::Vector2f(2,18));
    cursor_indicator.setFillColor(sf::Color::Yellow);
    setCursorPosition(cursor_indicator,window_width/2,(window_height/2)-45,0);

    string user_input_string;
    int user_input_string_length=0;

    bool game_start = false;


    sf::RenderWindow welcome(sf::VideoMode(window_width, window_height), "Minesweeper",sf::Style::Close);
    while (welcome.isOpen()) {

        // Event loop
        sf::Event event;
        while (welcome.pollEvent(event)) {
            // Close event
            if (event.type == sf::Event::EventType::Closed) {
                welcome.close();
                break;
            }

            //check for keyboard input to type characters for the name
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128) {
                    string current_char; //initializes a string for the current character typed in
                    current_char = current_char + static_cast<char>(event.text.unicode);

                    if (isalpha(current_char[0])) { //checks if the key pressed is a valid letter
                        if (user_input_string_length<10) {
                            if (user_input_string_length==0) {
                                user_input_string += std::toupper(current_char[0]);
                                user_typed_name.setString(user_input_string);
                                setTextPosition(user_typed_name,window_width/2,(window_height/2)-45);
                                user_input_string_length+=1;
                                setCursorPosition(cursor_indicator,window_width/2,(window_height/2)-45,user_input_string_length);
                            }
                            else {
                                user_input_string += std::tolower(current_char[0]);
                                user_typed_name.setString(user_input_string);
                                setTextPosition(user_typed_name,window_width/2,(window_height/2)-45);
                                user_input_string_length+=1;
                                setCursorPosition(cursor_indicator,window_width/2,(window_height/2)-45,user_input_string_length);
                            }
                        }
                    }
                }
            }

            if (event.type == sf::Event::KeyPressed) { //will check keyboard inputs
                if (event.key.code == sf::Keyboard::Backspace) { //when backspace is pressed, remove the last character from string if something is there.
                    if (user_input_string_length > 0) {
                        user_input_string.pop_back(); //removes last character
                        user_typed_name.setString(user_input_string);
                        setTextPosition(user_typed_name,window_width/2,(window_height/2)-45);
                        user_input_string_length-=1;
                        setCursorPosition(cursor_indicator,window_width/2,(window_height/2)-45,user_input_string_length);
                    }
                }

                if (event.key.code == sf::Keyboard::Enter) {
                    if (user_input_string_length>=1) {
                        game_start = true;
                        welcome.close();
                        break;
                    }
                }
            }
        }
        welcome.clear(sf::Color::Blue);
        welcome.draw(welcome_text);
        welcome.draw(enter_name_text);
        welcome.draw(user_typed_name);
        welcome.draw(cursor_indicator);
        welcome.display();
    }



    //Main Minesweeper game
    if (game_start == true) {
        // Load the texture files
        sf::Texture up_texture;
        sf::Texture down_texture;
        up_texture.loadFromFile("files/images/tile_hidden.png");
        down_texture.loadFromFile("files/images/tile_revealed.png");

        sf::Texture face_happy_texture;
        face_happy_texture.loadFromFile("files/images/face_happy.png");
        sf::Texture face_lose_texture;
        face_lose_texture.loadFromFile("files/images/face_lose.png");
        sf::Texture face_win_texture;
        face_win_texture.loadFromFile("files/images/face_win.png");

        sf::Texture debug_texture;
        debug_texture.loadFromFile("files/images/debug.png");
        sf::Texture pause_texture;
        pause_texture.loadFromFile("files/images/pause.png");
        sf::Texture play_texture;
        play_texture.loadFromFile("files/images/play.png");
        sf::Texture leaderboard_texture;
        leaderboard_texture.loadFromFile("files/images/leaderboard.png");

        sf::Texture mine_texture;
        mine_texture.loadFromFile("files/images/mine.png");
        vector<Mine_Sprite> mine_sprites;

        sf::Texture flag_texture;
        flag_texture.loadFromFile("files/images/flag.png");
        sf::Sprite flag_sprite = sf::Sprite(flag_texture);

        sf::Texture digit_texture;
        digit_texture.loadFromFile("files/images/digits.png");
        sf::Sprite digit_sprite = sf::Sprite(digit_texture);

        sf::Texture number_texture;


        Config.seekg(0,std::ios::beg);
        string cols;
        getline(Config,cols);
        string rows;
        getline(Config,rows);
        string mines;
        getline(Config,mines);
        int maxtilecount = stoi(rows)*stoi(cols);
        int mine_count = stoi(mines);
        int num_of_tiles_to_reveal = maxtilecount - mine_count;

        // Create the tiles and pushes them to the vector. Amount based the row and col numbers in the config file
        vector<Tile> tiles;

        //Starting game
        CreateBoard(tiles,window_height,window_width,maxtilecount,up_texture,down_texture);
        SetAdjacentTiles(tiles);

        GenerateMines(tiles,mine_count,mine_sprites,mine_texture);

        bool debug = false;
        int current_mine_count = mine_count;
        bool game_over = false;
        bool pause = false;
        bool win = false;
        bool leaderboard_opened = false;
        bool leaderboard_updated = false;
        bool pause_timer_win = false;
        bool hide_contents = false;

        Happy_Face_Button happyfacebutton = Happy_Face_Button(face_happy_texture,(stoi(cols)/2.0)*32-32,32*(stoi(rows)+0.5),face_lose_texture,face_win_texture);
        Debug_Button debugbutton = Debug_Button(debug_texture,(stoi(cols)*32)-304,32*(stoi(rows)+0.5));
        Play_Pause_Button playpausebutton = Play_Pause_Button(play_texture,pause_texture,(stoi(cols)*32)-240,32*(stoi(rows)+0.5));
        playpausebutton.click();
        Leaderboard_Button leaderboardbutton = Leaderboard_Button(leaderboard_texture,(stoi(cols)*32)-176,32*(stoi(rows)+0.5));

        int minutes;
        int seconds;

        Timer timer;
        timer.StartTimer();

        sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Minesweeper",sf::Style::Close);

        // Mainloop
        while (window.isOpen()) {

            // Event loop
            sf::Event event;
            while (window.pollEvent(event)) {
                // Close event
                if (event.type == sf::Event::EventType::Closed) {
                    window.close();
                    break;
                }

                // Click event
                if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                    auto click = event.mouseButton;
                        // See if the click intersects any tiles. If so, run their click() function
                        for (Tile& tile: tiles) {
                            // getGlobalBounds().contains(x, y) tests if (x, y) touches the sprite
                            if (tile.get_sprite().getGlobalBounds().contains(click.x, click.y) && game_over==false && pause==false && win==false && leaderboard_opened==false) {
                                tile.click(); // Click function as defined in the tile class
                            }
                        }
                        if (playpausebutton.get_sprite().getGlobalBounds().contains(click.x,click.y) && game_over==false && leaderboard_opened==false && win==false && leaderboard_opened == false) {
                            playpausebutton.click();
                            pause = !pause;
                            if (pause) {
                                timer.PauseTimer();
                                playpausebutton.SwitchPreviousPauseState();
                            }
                            else {
                                timer.StartTimer();
                                playpausebutton.SwitchPreviousPauseState();
                            }

                        }
                        if (debugbutton.get_sprite().getGlobalBounds().contains(click.x,click.y) && game_over==false && pause==false && win==false && leaderboard_opened == false) {
                            if (debug == false) {
                                debug = true;
                            }
                            else {
                                debug = false;
                            }
                        }
                        if (happyfacebutton.get_sprite().getGlobalBounds().contains(click.x,click.y) && leaderboard_opened==false) {
                            //will reset the game
                            mine_sprites.clear();
                            tiles.clear();
                            current_mine_count = mine_count;
                            happyfacebutton.change_texture("happy");
                            CreateBoard(tiles,window_height,window_width,maxtilecount,up_texture,down_texture);
                            SetAdjacentTiles(tiles);
                            GenerateMines(tiles,mine_count,mine_sprites,mine_texture);
                            game_over = false;
                            win = false;
                            leaderboard_updated = false;
                            pause_timer_win = false;
                            timer.ResetTimer();
                            timer.StartTimer();
                        }
                        if (leaderboardbutton.get_sprite().getGlobalBounds().contains(click.x,click.y) && leaderboard_opened==false) {
                            if (!playpausebutton.IsGamePaused()) {
                                playpausebutton.click();
                            }
                            timer.PauseTimer();
                            hide_contents = true;
                            leaderboard_opened = true;
                            for (int i=0 ; i<tiles.size() ;i++) {
                                tiles[i].set_texture_to_down();
                            }
                        }
                }

                if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
                    auto click = event.mouseButton;
                    int num_of_flagged_tiles = 0;
                    for (Tile& tile: tiles) {
                        // getGlobalBounds().contains(x, y) tests if (x, y) touches the sprite
                        if (tile.get_sprite().getGlobalBounds().contains(click.x, click.y) && game_over==false && pause==false && leaderboard_opened==false && win == false) {
                            tile.rightclick(); // Click function as defined in the tile class
                        }
                        if (tile.IsFlagged()) {
                            num_of_flagged_tiles += 1;
                        }
                    }
                    current_mine_count = mine_count - num_of_flagged_tiles;
                }
            }

            // Render loop
            window.clear(sf::Color::White);

            minutes = timer.getMinutes();
            seconds = timer.getSeconds();

            if (CheckIfWin(tiles,num_of_tiles_to_reveal,mine_count) && win == false) { //checks for win
                win = true;
                happyfacebutton.change_texture("win");
                leaderboard_opened = true;
                timer.PauseTimer();
                pause_timer_win = true;
                for (int i=0 ; i<tiles.size() ;i++) {
                    if (!tiles[i].CheckIfMine()) {
                        tiles[i].set_texture_to_down();
                    }
                    if (tiles[i].CheckIfMine() && !tiles[i].IsFlagged()) {
                        tiles[i].rightclick();
                    }
                }
                current_mine_count = 0;
            }

            for (Tile& tile: tiles) { //draws all tiles to the screen
                window.draw(tile.get_sprite());

            }

            for (int i=0 ; i<tiles.size() ; i++) { //will set a tile's texture based on whether or not it's revealed and the current pause state
                if (pause==false) {
                    if (!tiles[i].IsRevealed()) {
                        tiles[i].set_texture_to_up();
                    }
                    else if (tiles[i].IsRevealed()) {
                        tiles[i].set_texture_to_down();
                    }
                }
                else if (pause || leaderboard_opened) {
                    tiles[i].set_texture_to_down();
                }
            }

            for (int i=0 ; i<tiles.size() ; i++) { //draws the adjacent mine count for any tiles that are revealed
                if (tiles[i].IsRevealed() && tiles[i].get_adjacent_mines()>0 && pause == false && hide_contents == false) {
                    string filename = "files/images/number_" + to_string(tiles[i].get_adjacent_mines()) + ".png";
                    number_texture.loadFromFile(filename);
                    sf::Sprite number_sprite = sf::Sprite(number_texture);
                    number_sprite.setPosition(tiles[i].GetXPosition(),tiles[i].GetYPosition());
                    window.draw(number_sprite);
                }
            }

            for (int i=0 ; i<tiles.size() ; i++) { //draws any flagged tile
                if (tiles[i].IsFlagged() && pause == false && hide_contents == false) {
                    flag_sprite.setPosition(tiles[i].GetXPosition(),tiles[i].GetYPosition());
                    window.draw(flag_sprite);
                }
            }

            for (int i=0 ; i<tiles.size() ; i++) { //checks if any mine has been selected to constitute game over
                if (tiles[i].CheckForGameOver()) {
                    game_over = true;
                    timer.PauseTimer();
                    pause_timer_win = true;
                    happyfacebutton.change_texture("lose");
                }
            }


            if ((debug == true || game_over) && pause == false && hide_contents == false) { //draws all mines if the debug button is selected or game over
                for (int i=0 ; i<mine_sprites.size() ; i++) {
                    window.draw(mine_sprites[i].get_sprite());
                }
            }

            if (pause_timer_win == true) {
                timer.PauseTimer();
            }

            window.draw(happyfacebutton.get_sprite());
            window.draw(debugbutton.get_sprite());
            window.draw(playpausebutton.get_sprite());
            window.draw(leaderboardbutton.get_sprite());
            DrawCounter(3,current_mine_count,digit_sprite,window,stoi(rows),stoi(cols));
            DrawTimer(minutes,seconds,digit_sprite,window,stoi(rows),stoi(cols));
            window.display();

            if (leaderboard_opened) {
                pause = true;
                LeaderBoard_Window(font,leaderboard_width,leaderboard_height,playpausebutton,leaderboard_opened,win,user_input_string,minutes,seconds,leaderboard_updated,pause,timer, hide_contents);
            }

        }
    }

    return 0;
}