#pragma once
#ifdef UNICODE
#undef UNICODE					
#endif
#include <graphics.h>
#include <string>
#include <iostream>
#include <vector>

//定义玩家速度
#define		PLAYER_SPEED	3
//玩家高度
#define		PLAYER_WIDTH	80
//玩家宽度
#define		PLAYER_HEIGHT	80
//阴影宽度
#define		SHADOW_WIDTH	32
//敌人速度
#define		ENEMY_SPEED		2
//敌人宽度
#define		FRAME_WIDTH		80
//敌人高度
#define		FRAME_HEIGHT	80
//敌人阴影宽度
#define		ENEMY_SHADOW_WIDTH		48
//子弹
#define		RADIUS			10

//当前动画帧索引
int idx_current_anim = 0;

//动画帧总数
const int PLAYER_ANIM_NUM = 6;

//窗口宽度与高度
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

//Alpha透明度消除角色周围黑框
#pragma comment(lib,"Winmm.lib")
#pragma comment(lib , "MSIMG32.LIB")
//标识当前游戏是否开始
bool is_game_started = false;
//当前游戏是否正在运行
bool running = true;

void putimage_alpha(IMAGE* target, int x, int y, IMAGE* source)
{
	int w = source->getwidth();
	int h = source->getheight();
	HDC dstDC = GetImageHDC(target);
	HDC srcDC = GetImageHDC(source);

	// 结构体的第三个成员表示额外的透明度，0 表示全透明，255 表示不透明。
	BLENDFUNCTION bf = { AC_SRC_OVER , 0 , 255 , AC_SRC_ALPHA };
	AlphaBlend(dstDC, x, y, w, h, srcDC, 0, 0, w, h, bf);
}
//共享的数据，动画所使用的图集
class Atlas
{
public:
	Atlas(LPCTSTR path, int num) 
	{
		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}

	~Atlas()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
			delete frame_list[i];
	}
public:
	std::vector<IMAGE*> frame_list;
};

Atlas* atlas_player_left;
Atlas* atlas_player_right; 
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right;
//动画类，用于加载动画帧
class Animation
{
public:
	Animation(Atlas* atlas,int interval)
	{
		timer = 0;
		idx_frame = 0;
		interval_ms = 0;
		anim_atlas = atlas;
		interval_ms = interval;
	}

	~Animation()
	{
	}

	void Play(int x, int y, int delta)
	{
		timer += delta;
		if (timer >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
			timer = 0;
		}

		putimage_alpha(NULL, x, y, anim_atlas->frame_list[idx_frame]);
	}
private:
	//动画计时器，计时超过interval之后开启下一帧
	int timer;
	//动画的帧索引
	int idx_frame;
	int interval_ms;
private:
	Atlas* anim_atlas;
};
//玩家类，玩家的基本绘图操作
class Player
{
public:
	Player()
	{
		//玩家位置定义
		position.x = 500;
		position.y = 500;
		is_move_up = false;
		is_move_down = false;
		is_move_left = false;
		is_move_right = false;
		loadimage(&img_shadow, "img/shadow_player.png");
		//加载动画帧图片
		anim_left = new Animation(atlas_player_left,45);
		anim_right = new Animation(atlas_player_right,45);
	}

	~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg)
	{
		switch(msg.message)
		{
		case WM_KEYDOWN:
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = true;
				break;
			case VK_DOWN:
				is_move_down = true;
				break;
			case VK_LEFT:
				is_move_left = true;
				break;
			case VK_RIGHT:
				is_move_right = true;
				break;
			}
			break;
		case WM_KEYUP:
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = false;
				break;
			case VK_DOWN:
				is_move_down = false;
				break;
			case VK_LEFT:
				is_move_left = false;
				break;
			case VK_RIGHT:
				is_move_right = false;
				break;
			}
			break;
		}
	}

	//处理玩家移动
	void Move()
	{
		//利用向量知识将斜方向上与正方向上速度维持稳定
		double dir_x = is_move_right - is_move_left;
		double dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(PLAYER_SPEED * normalized_x);
			position.y += (int)(PLAYER_SPEED * normalized_y);
		}

		//校准玩家位置，防止玩家冲出窗口边界
		if (position.x < 0)position.x = 0;
		if (position.y < 0)position.y = 0;
		if (position.x + PLAYER_WIDTH > WINDOW_WIDTH)position.x = WINDOW_WIDTH - PLAYER_WIDTH;
		if (position.y + PLAYER_HEIGHT > WINDOW_HEIGHT)position.y = WINDOW_HEIGHT - PLAYER_HEIGHT;
	}

	//绘制玩家
	void Draw(int delta)
	{
		//让阴影图片居中
		int pos_shadow_x = position.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + PLAYER_HEIGHT - 8;
		putimage_alpha(NULL, pos_shadow_x, pos_shadow_y, &img_shadow);

		static bool facing_left = false;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;

		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}

	const POINT& GetPosition() const
	{
		return position;
	}
private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	//玩家位置定义
	POINT position;
	bool is_move_up;
	bool is_move_down;
	bool is_move_left;
	bool is_move_right;
};
//子弹类，子弹的基本属性
class Bullet
{
public:
	POINT position;

public:
	Bullet() 
	{
		position.x = 0;
		position.y = 0;
	}
	~Bullet() {}

	void Draw() const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	}
};
//敌人类，敌人的基本属性及操作
class Enemy
{
public:
	Enemy()
	{
		position.x = 0;
		position.y = 0;
		facing_left = false;
		alive = true;
		loadimage(&img_shadow, "img/shadow_enemy.png");
		//加载动画帧图片
		anim_left = new Animation(atlas_enemy_left, 45);
		anim_right = new Animation(atlas_enemy_right, 45);


		//敌人生成边界
		enum SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};
		//将敌人放置在地图边界外随机位置
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
			//上下边界敌人x坐标随机
		case SpawnEdge::Up:
			position.x = rand() % WINDOW_WIDTH;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH;
			position.y = FRAME_HEIGHT;
			//左右边界敌人y坐标随机
		case SpawnEdge::Left:
			position.x = -FRAME_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
		case SpawnEdge::Right:
			position.x = FRAME_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
		default:
			break;
		}
	}
	//检测子弹与敌人碰撞
	bool CheckBulletCollision(const Bullet& bullet)
	{
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}
	//检测敌人与玩家之间碰撞
	bool CheckPlayerCollision(const Player& player)
	{
		//将敌人等效为一个点
		POINT check_position = { position.x + FRAME_WIDTH / 2,position.y + FRAME_HEIGHT / 2 };
		bool is_overlap_x = check_position.x >= player.GetPosition().x && check_position.x <= player.GetPosition().x + PLAYER_WIDTH;
		bool is_overlap_y = check_position.y >= player.GetPosition().y && check_position.y <= player.GetPosition().y + PLAYER_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}
	//计算敌人需要移动的向量
	void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		double dir_x = player_position.x - position.x;
		double dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(ENEMY_SPEED * normalized_x);
			position.y += (int)(ENEMY_SPEED * normalized_y);
		}

		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;
	}
	void Draw(int delta)
	{
		//让阴影图片居中
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - ENEMY_SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + FRAME_WIDTH - 35;
		putimage_alpha(NULL, pos_shadow_x, pos_shadow_y, &img_shadow);

		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);
	}
	~Enemy()
	{
		delete anim_left;
		delete anim_right;
	}
	void Hurt()
	{
		alive = false;
	}
	bool CheckAlive()
	{
		return alive;
	}
private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position;
	bool facing_left;
	bool alive;
};
//按钮类，按钮的实现
class Button
{
public:
	Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
	{
		status = Status::Idle;

		region = rect;

		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}
	~Button() 
	{
	}
	void Draw()
	{
		switch (status)
		{
		case Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}
	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
				status = Status::Hovered;
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
				status = Status::Idle;
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
				status = Status::Pushed;
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
				OnClick();
			break;
		default:
			break;
		}
	}
protected:
	virtual void OnClick() = 0;

private:
	enum Status
	{
		Idle = 0,
		Hovered,
		Pushed
	};
private:
	RECT region;
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status;
private:
	bool CheckCursorHit(int x, int y)
	{
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};
//继承自父类按钮的子类
class StartGameButton : public Button
{
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed) :Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
	{
	}
	~StartGameButton()
	{
	}
protected:
	void OnClick()
	{
		is_game_started = true;

		mciSendString("play bgm repeat from 0", NULL, 0, NULL);
	}
};
class QuitGameButton :public Button
{
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed) :Button(rect, path_img_idle, path_img_hovered, path_img_pushed)
	{
	}
	~QuitGameButton()
	{
	}
protected:
	void OnClick()
	{
		running = false;
	}
};
//生成敌人，内置简单计数器，定时添加敌人
void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
		enemy_list.push_back(new Enemy());
}
//显示子弹
void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player)
{
	//径向速度
	const double RADIAL_SPEED = 0.0045;
	//切向速度
	const double TANGENT_SPEED = 0.0055;
	//子弹间的弧度间隔
	double radian_interval = 2 * 3.14159 / bullet_list.size();
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;
		bullet_list[i].position.x = player_position.x + FRAME_WIDTH / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + FRAME_HEIGHT / 2 + (int)(radius * cos(radian));
	}
}
//绘制玩家得分
void DrawPlayerScore(int score)
{
	static TCHAR text[64];
	_stprintf_s(text, "当前玩家得分：%d", score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(10, 10, text);
}
int main()
{
	initgraph(1280, 720);

	atlas_player_left = new Atlas("img/player_left_%d.png", 6);
	atlas_player_right = new Atlas("img/player_right_%d.png", 6);
	atlas_enemy_left = new Atlas("img/enemy_left_%d.png", 6);
	atlas_enemy_right = new Atlas("img/enemy_right_%d.png", 6);

	mciSendString("open mus/bgm.mp3 alias bgm", NULL, 0, NULL);
	mciSendString("open mus/hit.wav alias hit", NULL, 0, NULL);

	//得分机制
	int score = 0;
	Player player;
	ExMessage msg;
	IMAGE img_menu;
	IMAGE img_background;
	std::vector<Enemy*> enemy_list;
	std::vector<Bullet> bullet_list(3);

	RECT region_btn_start_game, region_btn_quit_game;

	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_start_game.right = region_btn_start_game.left + BUTTON_WIDTH;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_quit_game.right = region_btn_quit_game.left + BUTTON_WIDTH;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

	StartGameButton btn_start_game = StartGameButton(region_btn_start_game, "img/ui_start_idle.png", "img/ui_start_hovered.png", "img/ui_start_pushed.png");
	QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game, "img/ui_quit_idle.png", "img/ui_quit_hovered.png", "img/ui_quit_pushed.png");

	loadimage(&img_menu, "img/menu.png");
	loadimage(&img_background, "img/background.png");

	BeginBatchDraw();

	while (running)
	{
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg))
		{
			if(is_game_started)
				player.ProcessEvent(msg);
			else
			{
				btn_start_game.ProcessEvent(msg);
				btn_quit_game.ProcessEvent(msg);
			}
		}
		if (is_game_started)
		{
			player.Move();
			UpdateBullets(bullet_list, player);
			TryGenerateEnemy(enemy_list);
			//遍历数组中每个敌人，依次调用方法
			for (int i = 0; i < enemy_list.size(); i++)
				enemy_list[i]->Move(player);
			//检测角色与敌人碰撞
			for (int i = 0; i < enemy_list.size(); i++)
			{
				if (enemy_list[i]->CheckPlayerCollision(player))
				{
					static TCHAR text[128];
					_stprintf_s(text, "最终得分：%d !", score);
					MessageBox(GetHWnd(), "扣1", "游戏结束", MB_OK);
					running = false;
					break;
				}
			}
			//子弹与敌人碰撞逻辑
			for (int i = 0; i < enemy_list.size(); i++)
			{
				for (int j = 0; j < bullet_list.size(); j++)
				{
					if (enemy_list[i]->CheckBulletCollision(bullet_list[j]))
					{
						mciSendString("play hit from 0", NULL, 0, NULL);
						enemy_list[i]->Hurt();
						score++;
					}
				}
			}
			//移除已消灭的敌人
			for (size_t i = 0; i < enemy_list.size(); i++)
			{
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive())
				{
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;
				}
			}
		}
		cleardevice();
		if(is_game_started)
		{
			putimage(0, 0, &img_background);
			player.Draw(1000 / 144);
			for (int i = 0; i < enemy_list.size(); i++)
				enemy_list[i]->Draw(1000 / 144);
			for (int i = 0; i < bullet_list.size(); i++)
				bullet_list[i].Draw();
			DrawPlayerScore(score);
		}
		else
		{
			putimage(0, 0, &img_menu);
			btn_start_game.Draw();
			btn_quit_game.Draw();
		}
		FlushBatchDraw();

		//动态延时，稳定帧率
		DWORD end_time = GetTickCount();                
		DWORD delta_time = end_time - start_time;
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}
	}

	delete atlas_player_left;
	delete atlas_player_right;
	delete atlas_enemy_left;
	delete atlas_enemy_right;

	EndBatchDraw();

	return 0;

}