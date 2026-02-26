#pragma once
#include <JuceHeader.h>

struct AnimationDef {
    juce::String name;
    int frameCount;
};

struct CharacterDef {
    juce::String categoryName;
    std::vector<juce::String> filenames; // Updated to support multi-sheet
    std::vector<AnimationDef> anims;
    bool isGridSprite = false;           // Engine flag
};

class SpriteDatabase
{
public:
   static std::vector<CharacterDef> getDatabase()
    {
        std::vector<CharacterDef> db;

        // Updated Helper Lambda
        auto addChar = [&](juce::String name, std::vector<juce::String> files, bool isGrid) {
            CharacterDef c; 
            c.categoryName = name; 
            c.filenames = files; 
            c.isGridSprite = isGrid;
            return c;
        };
      // 1. CAT
        auto cat = addChar("Cat", {
            "Cat0.png", "Cat1.png", "Cat2.png", "Cat3.png", 
            "Cat4.png", "Cat5.png", "Cat6.png", "Cat7.png"
        }, true); // <-- Changed to true
        cat.anims = {
            {"Cat Boom", 12}, {"Maxwell Rock", 48}, {"Vibe Cat", 81}, {"Party Cat", 69}, {"Jumping Cat", 111},
            {"Cat Moment", 4}, {"Cat Grab", 92}, {"Gangnam Cat", 48}, {"Wave Cat", 20}, {"Maxwell Spin", 56}
        };
        db.push_back(cat);

        // 2. DOG
        auto dog = addChar("Dog", {
            "Dog0.png", "Dog1.png", "Dog2.png", "Dog3.png", 
            "Dog4.png", "Dog5.png", "Dog6.png"
        }, true); // <-- Changed to true
        dog.anims = {
            {"6O163", 4}, {"Doge", 10}, {"Turntable", 10}, {"Doge 2", 12}, {"Pug", 65},
            {"PUGS!!!", 63}, {"Dug", 112}, {"Twerk", 100}, {"Party Dog", 90}, {"Spin", 16}
        };
        db.push_back(dog);

        // 3. FROG
        auto frog = addChar("Frog", {"Frog.png"}, false);
        frog.anims = {
            {"PepeVibe", 5}, {"PepeJam", 6}, {"PepePls", 22}, {"PepeSadDance", 15}, {"8BitPepe", 12},
            {"PepeLeBron", 12}, {"FF Frog", 10}, {"Ditto Frog", 12}, {"FrogShroom", 8}, {"MLG Frog", 9}
        };
        db.push_back(frog);

        // 4. OTHER ANIMALS
        auto other = addChar("Other Animals", {
            "OtherAnimals0.png", "OtherAnimals1.png", "OtherAnimals2.png", "OtherAnimals3.png", 
            "OtherAnimals4.png", "OtherAnimals5.png", "OtherAnimals6.png", "OtherAnimals7.png", "OtherAnimals8.png"
        }, true);
        other.anims = {
            {"Brazil Dog Vaporwave", 148}, {"Brazil Dog", 57}, {"Vibe Dog", 89}, {"Butter Dog", 63}, {"Polish Cow", 21},
            {"Club Penguin", 32}, {"Pusheen Capybara", 16}, {"Goose", 128}, {"Rainbow Roach", 46}, {"Rainbow Parrot", 10}
        };
        db.push_back(other);

        // 5. NYAN CAT
        auto nyan = addChar("Nyan Cat", {"Nyan Cat.png"}, false);
        nyan.anims = {
            {"Nyan Cat Clean", 12}, {"Nyan Cat Original", 8}, {"Nyan Real Cat", 11}, {"Nyan Pikachu", 4}, {"Donut Cat", 4},
            {"Rainbow Tail", 4}, {"Nyan Gato", 8}, {"Nyan Cat Purple", 12}, {"nyanwave", 16}, {"Nyan Glitch", 12}
        };
        db.push_back(nyan);

        // 6. LINK
        auto link = addChar("Link", {"Link.png"}, false);
        link.anims = {
            {"Link", 8}, {"Crumply", 8}, {"Pixely", 8}, {"Link", 8}, {"Colorful", 8},
            {"Fire", 8}, {"Water", 8}, {"Shadow", 8}, {"Purple", 8}, {"Upside Down", 8}
        };
        db.push_back(link);

         // 7. VIDEO GAME
        auto game = addChar("Video Game", {
            "VideoGame0.png", "VideoGame1.png", "VideoGame2.png", "VideoGame3.png"
        }, true);
        game.anims = {
            {"Hadouken", 14}, {"Ryu Pose", 23}, {"Ryu Idle", 6}, {"Ken Attack", 90}, {"Souls", 56},
            {"Pot Head", 2}, {"Eevee", 8}, {"Kirby", 15}, {"Amongus Thiccc", 6}, {"Ditto", 4}
        };
        db.push_back(game);

        // 8. TWICE
        auto twice = addChar("Twice", {
            "Twice0.png", "Twice1.png", "Twice2.png", "Twice3.png", "Twice4.png"
        }, true);
        twice.anims = {
            {"Jihyo", 36}, {"Twice", 36}, {"Tzuyu", 44}, {"Momo", 50}, {"Dahyun", 68},
            {"Sana", 19}, {"Jihyo 2", 21}, {"Sana 2", 32}, {"Mina", 35}, {"Momo 2", 16}
        };
        db.push_back(twice);

       // 9. ANIME
        auto anime = addChar("Anime", {
            "Anime0.png", "Anime1.png", "Anime2.png"
        }, true);
        anime.anims = {
            {"Chinatsu Yoshikawa", 8}, {"Akari Akaza", 8}, {"Cat Girl", 6}, {"Panda", 4}, {"Yay", 10},
            {"Haruhi", 14}, {"Konosuba", 17}, {"Aqua", 18}, {"Invader Girl", 127}, {"Finger Spin", 4}
        };
        db.push_back(anime);

        // 10. ANIME 2
        auto anime2 = addChar("Anime 2", {
            "Anime2-0.png", "Anime2-1.png", "Anime2-2.png", "Anime2-3.png", "Anime2-4.png"
        }, true); 
        anime2.anims = {
            {"Yui", 20}, {"Chika 1", 20}, {"Chika 2", 21}, {"Chika 3", 80}, {"ME!ME!ME!", 20},
            {"Zero Two", 20}, {"Baggy Clothes Dance", 36}, {"Chibi Dance", 50}, {"oki doki", 69}, {"Esiledoodles Cora", 16}
        };
        db.push_back(anime2);

        // 11. FRUITY CHAN
        auto fruity = addChar("Fruity Chan", {"Fruity Chan.png"}, false);
        fruity.anims = {
            {"Waiting", 8}, {"Stepping", 8}, {"Jumping", 8}, {"Zombie", 8}, {"Waving", 8},
            {"Hula", 8}, {"Windmill", 8}, {"Zitabata", 8}, {"Dervish", 8}, {"Held", 8}
        };
        db.push_back(fruity);
        
        // 12. CARS 
        auto cars = addChar("Cars", {
            "Cars0.png", "Cars1.png", "Cars2.png", "Cars3.png", 
            "Cars4.png", "Cars5.png", "Cars6.png", "Cars7.png"
        }, true);
        cars.anims = {
            {"Spin White", 53}, {"Spin Blue", 53}, {"Spin Red", 53}, {"Spin Multi", 136}, {"Spin Low Poly", 51},
            {"Burnout", 45}, {"Insert Coin(s)", 29}, {"Retro", 77}, {"Burning Rubber", 20}, {"Corners", 59}
        };
        db.push_back(cars);

        // 13. OTHER DANCE
        auto dance = addChar("Other Dance", {
            "OtherDance0.png", "OtherDance1.png", "OtherDance2.png", 
            "OtherDance3.png", "OtherDance4.png", "OtherDance5.png"
        }, true);
        dance.anims = {
            {"Snoop", 58}, {"Jojo Toture Dance", 12}, {"Toothless", 122}, {"Miku", 8}, {"Thanos Twerk", 34},
            {"Elaine", 11}, {"Squidward Talent Show Dance", 52}, {"OG Dancing Baby", 52}, {"Car Shearer", 47}, {"Leek Spin", 4}
        };
        db.push_back(dance);

        return db;
    }
};