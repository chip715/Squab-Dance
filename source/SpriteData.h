#pragma once
#include <JuceHeader.h>

struct AnimationDef {
    juce::String name;
    int frameCount;
};

struct CharacterDef {
    juce::String categoryName;     // e.g., "Cat"
    juce::String filename;         // e.g., "Cat.png"
    std::vector<AnimationDef> anims;
};

class SpriteDatabase
{
public:
    static std::vector<CharacterDef> getDatabase()
    {
        std::vector<CharacterDef> db;

        // --- Helper to make adding easier ---
        auto addChar = [&](juce::String name, juce::String file) {
            CharacterDef c; c.categoryName = name; c.filename = file; return c;
        };

        // 1. CAT
        auto cat = addChar("Cat", "Cat.png");
        cat.anims = {
            {"Cat Boom", 12}, {"Maxwell Rock", 48}, {"Vibe Cat", 82}, {"Party Cat", 69}, {"Jumping Cat", 111},
            {"Cat Moment", 4}, {"Cat Grab", 92}, {"Gangnam Cat", 48}, {"Wave Cat", 20}, {"Maxwell Spin", 57}
        };
        db.push_back(cat);

        // 2. DOG
        auto dog = addChar("Dog", "Dog.png");
        dog.anims = {
            {"6O163", 4}, {"Doge Turntable", 10}, {"Doge 2", 10}, {"Pug PUGS!!!", 12}, {"Dug Twerk", 65},
            {"Party Dog", 63}, {"Spin", 112}, {"Row 7", 100}, {"Row 8", 90}, {"Row 9", 16} // Added generic names for missing ones
        };
        db.push_back(dog);

        // 3. FROG
        auto frog = addChar("Frog", "Frog.png");
        frog.anims = {
            {"PepeVibe", 5}, {"PepeJam", 6}, {"PepePls", 22}, {"PepeSadDance", 15}, {"8BitPepe", 12},
            {"PepeLeBron", 12}, {"FF Frog", 10}, {"Ditto Frog", 12}, {"FrogShroom", 8}, {"MLG Frog", 9}
        };
        db.push_back(frog);

        // 4. OTHER ANIMALS
        auto other = addChar("Other Animals", "Other Animals.png");
        other.anims = {
            {"Brazil Dog Vaporwave", 148}, {"Brazil Dog", 57}, {"Vibe Dog", 89}, {"Butter Dog", 63}, {"Barbie Dog", 21},
            {"Club Penguin", 32}, {"Pusheen Capybara", 16}, {"Goose", 128}, {"Rainbow Roach", 46}, {"Rainbow Parrot", 10}
        };
        db.push_back(other);

        // 5. NYAN CAT
        auto nyan = addChar("Nyan Cat", "Nyan Cat.png");
        nyan.anims = {
            {"Nyan Cat Clean", 12}, {"Nyan Cat Original", 8}, {"Nyan Real Cat", 11}, {"Nyan Pikachu", 4}, {"Donut Cat", 4},
            {"Rainbow Tail", 4}, {"Nyan Gato", 8}, {"Nyan Cat Purple", 12}, {"nyanwave", 16}, {"Nyan Glitch", 12}
        };
        db.push_back(nyan);

        // 6. LINK
        auto link = addChar("Link", "Link.png");
        link.anims = {
            {"Link Crumply", 8}, {"Pixely Link", 8}, {"Colorful", 8}, {"Fire", 8}, {"Water", 8},
            {"Shadow", 8}, {"Purple", 8}, {"Upside Down", 8}, {"Row 8", 8}, {"Row 9", 8}
        };
        db.push_back(link);

        // 7. VIDEO GAME
        auto game = addChar("Video Game", "Video Game.png");
        game.anims = {
            {"Hadouken", 14}, {"Ryu Pose", 23}, {"Ryu Idle", 6}, {"Ken Attack", 90}, {"Souls", 56},
            {"Pot Head", 2}, {"Eevee", 8}, {"Kirby", 15}, {"Amongus Thiccc", 6}, {"Ditto", 4}
        };
        db.push_back(game);

        // 8. TWICE
        auto twice = addChar("Twice", "Twice.png");
        twice.anims = {
            {"Jihyo", 36}, {"Twice", 36}, {"Tzuyu", 44}, {"Momo", 50}, {"Dahyun", 68},
            {"Sana", 19}, {"Jihyo 2", 21}, {"Sana 2", 32}, {"Mina", 35}, {"Momo 2", 16}
        };
        db.push_back(twice);

        // 9. ANIME
        auto anime = addChar("Anime", "Anime.png");
        anime.anims = {
            {"Chinatsu Yoshikawa", 8}, {"Akari Akaza", 8}, {"Cat Girl", 6}, {"Panda Yay", 4}, {"Haruhi", 10},
            {"Konosuba", 14}, {"Aqua", 17}, {"Invader Girl", 18}, {"Finger Spin", 127}, {"Row 9", 4}
        };
        db.push_back(anime);

        // 10. ANIME 2
        auto anime2 = addChar("Anime 2", "Anime 2.png");
        anime2.anims = {
            {"Yui", 20}, {"Chika 1", 20}, {"Chika 2", 21}, {"Chika 3", 80}, {"ME!ME!ME!", 20},
            {"Zero Two", 20}, {"Baggy Clothes", 36}, {"Chibi Dance", 50}, {"oki doki", 69}, {"Esiledoodles", 16}
        };
        db.push_back(anime2);

        // 11. FRUITY CHAN
        auto fruity = addChar("Fruity Chan", "Fruity Chan.png");
        fruity.anims = {
            {"Waiting", 8}, {"Stepping", 8}, {"Jumping", 8}, {"Zombie", 8}, {"Waving", 8},
            {"Hula", 8}, {"Windmill", 8}, {"Zitabata", 8}, {"Dervish", 8}, {"Held", 8}
        };
        db.push_back(fruity);
        
        // 12. CARS (You provided data but no names, so using placeholders)
        auto cars = addChar("Cars", "Cars.png");
        cars.anims = {
            {"Car 1", 53}, {"Car 2", 53}, {"Car 3", 53}, {"Car 4", 136}, {"Car 5", 51},
            {"Car 6", 45}, {"Car 7", 29}, {"Car 8", 77}, {"Car 9", 20}, {"Car 10", 59}
        };
        db.push_back(cars);

        // 13. OTHER DANCE
        auto dance = addChar("Other Dance", "Other Dance.png");
        dance.anims = {
            {"Snoop", 58}, {"Jojo Toture", 12}, {"Toothless", 122}, {"Miku", 8}, {"Thanos Twerk", 34},
            {"Elaine", 11}, {"Squidward", 52}, {"OG Dancing Baby", 52}, {"Car Shearer", 47}, {"Leek Spin", 4}
        };
        db.push_back(dance);

        return db;
    }
};