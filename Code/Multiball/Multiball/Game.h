
#include <SDL.h>
#include <vector>
#include <random>

#define INITIAL_BALLS 2

struct Vector2;
struct Ball;
struct Paddle;

constexpr float paddle_initial_offset = 20.f;

enum Paddles
{
    Paddle_Height = 100,
    Paddle_Width = 15,
    Paddle_Speed = 500,
};

enum Balls
{
    Ball_Thickness = 15,
    Ball_MinSpeed = 100,
    Ball_MaxSpeed = 300
};

enum Walls
{
    Wall_Thickness = 50
};

struct Vector2{
    float x;
    float y;
};

struct Paddle
{
    Vector2 pos;
    float speed;
    int direction;
    int height;
    int width;

    [[nodiscard]] SDL_Rect GetBounds() const
    {
        return {
                static_cast<int>(pos.x - static_cast<float>(width) / 2.f),
                static_cast<int>(pos.y - static_cast<float>(height) / 2.f),
                width,
                height
        };
    }

    void UpdatePaddlePosition(int native_height, float deltaTime)
    {
        if(direction != 0)
        {
            pos.y += static_cast<float>(direction) * speed * deltaTime;

            auto minY = static_cast<float>(static_cast<int>(static_cast<float>(height) / 2.0f) + Wall_Thickness);
            auto maxY = static_cast<float>(native_height - static_cast<int>(static_cast<float>(height) / 2.0f) - Wall_Thickness);

            if(pos.y < minY) pos.y = minY;
            else if(pos.y > maxY) pos.y = maxY;
        }
    }
};

struct Ball
{
    Vector2 pos;
    Vector2 vel;
    int thickness;

    [[nodiscard]] SDL_Rect GetBounds() const
    {
        return {
            static_cast<int>(pos.x - static_cast<float>(thickness) / 2.f),
            static_cast<int>(pos.y - static_cast<float>(thickness) / 2.f),
            thickness,
            thickness
        };
    }

    int UpdateBallPosition(Paddle paddles[], int native_width, int native_height, float deltaTime)
    {
        pos.x += vel.x * deltaTime;
        pos.y += vel.y * deltaTime;

        //collide with the top or bottom wall
        if((pos.y - static_cast<float>(thickness) / 2.f <= static_cast<float>(Wall_Thickness) && vel.y < 0.f) ||
           (pos.y + static_cast<float>(thickness) / 2.f >= static_cast<float>(native_height - Wall_Thickness) && vel.y > 0.f))
            vel.y *= -1.f;

        for(int i = 0; i < 2; i++)
        {
            auto paddle = paddles[i];
            float diff = paddle.pos.y - pos.y;
            // Take absolute value of difference
            diff = (diff > 0.0f) ? diff : -diff;
            if(
                    // Our y-difference is small enough
                    diff <= static_cast<float>(paddle.height) / 2.0f &&

                    // We are in the correct x-position
                    (
                            (
                            //ball is moving left and the ball's left edge is to the left of the right edge of the paddle, and the ball's left edge is to the right of the left edge of the paddle
                            vel.x < 0.0f &&
                            pos.x - static_cast<float>(thickness) / 2.f <= paddle.pos.x + static_cast<float>(paddle.width) / 2.f &&
                            pos.x - static_cast<float>(thickness) / 2.f >= paddle.pos.x - static_cast<float>(paddle.width) / 2.f
                            )
                        ||
                            (
                             //ball is moving right and the ball's right edge is to the right of the left edge of the paddle, and the ball's right edge is to the left of the right edge of the paddle
                             vel.x > 0.f &&
                             pos.x + static_cast<float>(thickness) / 2.f >= paddle.pos.x - static_cast<float>(paddle.width) / 2.f &&
                             pos.x + static_cast<float>(thickness) / 2.f <= paddle.pos.x + static_cast<float>(paddle.width) / 2.f

                             )
                     )
            )
            {
                vel.x *= -1.0f;
            }
        }

        //if the ball went off the left of the screen
        if(pos.x + static_cast<float>(thickness) / 2.f < static_cast<float>(-thickness))
        {
            return -1;
        }

        //if the ball went off the right of the screen
        if(pos.x - static_cast<float>(thickness) / 2.f > static_cast<float>(native_width + thickness))
        {
            return 1;
        }

        return 0;
    }
};

class Game
{
public:
	Game()
    {
        mWindow = nullptr;
        mRenderer = nullptr;

        mIsRunning = false;

        mTicksCount = 0;

        paddles[0] = {};
        paddles[1] = {};

        native_height = 0;
        native_width = 0;
    }

    bool Initialize();

	void RunLoop();

	void Shutdown();

private:
	void ProcessInput();
	void UpdateGame();
	void GenerateOutput();

	SDL_Window* mWindow;
    SDL_Renderer* mRenderer;

	bool mIsRunning;
    Uint32 mTicksCount;

    int native_width;
    int native_height;

    /** Paddle and ball stuff, not general to "Game" **/
    std::vector<Ball> balls;
    int score[2]{0, 0};

    Paddle paddles[2]{};
    [[nodiscard]] Ball GenerateNewBall() const
    {
        Ball b = {};
        b.pos = {static_cast<float>(native_width) / 2.f, static_cast<float>(native_height) / 2.f};
        b.vel = {RandomFloatInRange(Ball_MinSpeed, Ball_MaxSpeed), RandomFloatInRange(Ball_MinSpeed, Ball_MaxSpeed)};
        b.thickness = Balls::Ball_Thickness;
        return b;
    }

    static float RandomFloatInRange(float lower, float upper)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> rangeSelector(0, 1);
        int selectedRange = rangeSelector(gen);

        if (selectedRange == 0) {
            // Generate random float between min1 and max1
            std::uniform_real_distribution<float> dis(-upper, -lower);
            return dis(gen);
        } else {
            // Generate random float between min2 and max2
            std::uniform_real_distribution<float> dis(lower, upper);
            return dis(gen);
        }
    }

    SDL_TimerID timer_spawnBalls = 0;
    static Uint32 SpawnRandomBall_callback(Uint32 interval, void* param);
    void SpawnRandomBall_handle();
};