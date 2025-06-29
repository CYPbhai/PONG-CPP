#define is_down(b) input->buttons[b].is_down
#define pressed(b) (input->buttons[b].is_down && input->buttons[b].changed)
#define released(b) (!input->buttons[b].is_down && input->buttons[b].changed)

float player_1_p, player_1_dp, player_2_p, player_2_dp;
float arena_half_size_x = 85.f, arena_half_size_y = 45.f;
float player_half_size_x = 2.5f, player_half_size_y = 12.f;
float ball_p_x, ball_p_y, ball_dp_x = 130, ball_dp_y, ball_half_size = 1;

int player_1_score = 0, player_2_score = 0;

internal void
reset_game()
{
	player_1_p = 0.f;
	player_1_dp = 0.f;
	player_2_p = 0.f;
	player_2_dp = 0.f;
	ball_p_x = 0.f;
	ball_p_y = 0.f;
	ball_dp_x = 130.f;
	ball_dp_y = 0.f;
	player_1_score = 0;
	player_2_score = 0;
}

internal bool
aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y,
	float p2x, float p2y, float hs2x, float hs2y)
{
	return (p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y - hs1y < p2y + hs2y);
}

internal void
simulate_player(float *p, float *dp, float dpp, float dt)
{

	dpp -= *dp * 10.f;
	*p = *p + *dp * dt + dpp * dt * dt * .5f;
	*dp = *dp + dpp * dt;

	if (*p + player_half_size_y > arena_half_size_y)
	{
		*p = arena_half_size_y - player_half_size_y;
		*dp = 0.f;
	}
	else if (*p - player_half_size_y < -arena_half_size_y)
	{
		*p = -arena_half_size_y + player_half_size_y;
		*dp = 0.f;
	}
}

enum Gamemode
{
	GM_MENU,
	GM_GAMEPLAY
};

Gamemode current_gamemode;
int hot_button;
bool enemy_is_ai;

internal void
simulate_game(Input* input, float dt)
{
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0xffaa33);
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0xff5500);

	if (pressed(BUTTON_ESCAPE))
	{
		if (current_gamemode == GM_GAMEPLAY)
		{
			current_gamemode = GM_MENU;
			hot_button = 0;
			reset_game();
		}
		else if(current_gamemode == GM_MENU)
			running = false;
	}

	if (current_gamemode == GM_GAMEPLAY)
	{
		float player_1_ddp = 0.f;

		if (!enemy_is_ai) 
		{
			if (is_down(BUTTON_UP))    player_1_ddp += 1500;
			if (is_down(BUTTON_DOWN))  player_1_ddp -= 1500;
		}
		else
		{
			player_1_ddp = (ball_p_y - player_1_p) * 100.f;
			if (player_1_ddp > 975.f) player_1_ddp = 975;
			if (player_1_ddp < -975) player_1_ddp = -975;
		}

		float player_2_ddp = 0.f;
		if (is_down(BUTTON_W)) player_2_ddp += 1500;
		if (is_down(BUTTON_S)) player_2_ddp -= 1500;

		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
		simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);

		// Ball simulation
		{
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;

			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y))
			{
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1;
				ball_dp_y = (ball_p_y - player_1_p) * 2 + player_1_dp * .75f;
			}
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y))
			{
				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x *= -1;
				ball_dp_y = (ball_p_y - player_2_p) * 2 + player_2_dp * .75f;
			}

			if (ball_p_y + ball_half_size > arena_half_size_y)
			{
				ball_p_y = arena_half_size_y - ball_half_size;
				ball_dp_y *= -1;
			}
			else if (ball_p_y - ball_half_size < -arena_half_size_y)
			{
				ball_p_y = -arena_half_size_y + ball_half_size;
				ball_dp_y *= -1;
			}

			if (ball_p_x + ball_half_size > arena_half_size_x)
			{
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_1_score++;
			}
			else if (ball_p_x - ball_half_size < -arena_half_size_x)
			{
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_2_score++;
			}
		}

		draw_number(player_1_score, -10, 40, 1.f, 0xbbffbb);
		draw_number(player_2_score, 10, 40, 1.f, 0xbbffbb);

		// Rendering
		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);

		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0xff0000);
		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0xff0000);
	}
	else
	{
		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT))
		{
			hot_button = !hot_button;
		}
		if (pressed(BUTTON_ENTER))
		{
			current_gamemode = GM_GAMEPLAY;
			enemy_is_ai = hot_button ? 0 : 1;
		}
		if (hot_button == 0) 
		{
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xff0000);
			draw_text("MULTIPLAYER", 20, -10, 1, 0xaaaaaa);
		}
		else 
		{
			draw_text("SINGLE PLAYER", -80, -10, 1, 0xaaaaaa);
			draw_text("MULTIPLAYER", 20, -10, 1, 0xff0000);
		}
		draw_text("PONG", -40, 30, 4, 0xffffff);
	}
}