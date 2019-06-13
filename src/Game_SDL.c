/*
 ============================================================================
 Name        : 05_sprite_with_transparancy.c
 Author      : Riapolov Mikhail
 Version     :
 Copyright   : Use at your own risk
 Description : Чтобы поменять разрешение окна, откройте файл "save_game.txt"
 	 	 	   и поменяйти в первой строке первые два числа (default: 1280 720).
 	 	 	   Для передвижения используйте стрелки.
 	 	 	   a - анимация атаки
 	 	 	   f - добавление предмета (дерево)
 	 	 	   d - удаление предмета (дерево)
 	 	 	   r - увеличение насыщенности красного цвета
 	 	 	   g - увеличение насыщенности зелёного цвета
 	 	 	   b - увеличение насыщенности синего цвета
 	 	 	   s - сохранение настроек
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#if defined(__MINGW32__)
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#endif

// Размеры окна для вывода
int SCREEN_WIDTH = 1600;
int SCREEN_HEIGHT = 900;

uint8_t red = 0xFF, green = 0xFF, blue = 0xFF;

// путь к файлу с картинкой, содержащей файлы движения
static char sprite[] = "resources/adventurer.png";
static char wooden[] = "resources/wooden.png";
static char textMenu[] = "resources/alphabet.png";
// увеличение выводимых спрайтов (в данном примере исходно 32х32)
int scale = 4;

// Функция для инициализации библиотеки и создания окна
int initSDL();
// функция для проведения завершения работы библиотеки и освобождения всех ресурсов
void closeSDL();
void MenuMain();
// загрузка изображения
SDL_Texture* loadImage(char imagePath[]);

// Указатель на структуру, описывающую окно для вывода графики
SDL_Window* window = NULL;
// Указательн на структуру, описывающую механизм отрисовки в окне с применением аппаратного ускорения
SDL_Renderer* renderer = NULL;
// указатель на поверхность (surface), на которой мы будем рисовать и выводить её в окне
SDL_Surface* screenSurface = NULL;

// Переенные для хранения анимации персонажа
SDL_Texture* sprite_sheet;
SDL_Texture* wooden_sheet;
SDL_Texture* text_sheet;
// текущий кадр анимации
int anim_phase = 0;
// фаза анимации (бег, стояние на месте ...)
int anim_type = 0;
// количество кадров анимации для данного типа
int anim_phase_max[16] = { 13, 8, 10, 10, 10, 6, 4, 7, 13, 8, 10, 10, 10, 6, 4,
		7 };
// Кол-во букв в каждой строке
int text[6] = {7, 6, 6, 6, 6, 2};
// время с предыдущего кадра анимации
uint32_t last_frame;
// время в мс на 1 кадр
#define frame_time  60

// Структура для объекта
typedef struct things {
	int x;
	int y;
	struct things *next;
} things;

// Переменная для отслеживания кол-ва предметов
int bug_delete = 0;

things* getLast(things *last) {
	if (last == NULL) {
		return NULL;
	}
	while (last->next) {
		last = last->next;
	}
	return last;
}
// Функция добавления предмета в рандомное место
void pushThing(things* head){
	things* el = getLast(head);
	things* tmp = (things*)malloc(sizeof(things));
	tmp->x = rand() % SCREEN_WIDTH;
	tmp->y = rand() % SCREEN_HEIGHT;
	tmp->next = NULL;
	el->next = tmp;
	// Увеличиваем счётчик кол-ва предметов
	bug_delete = bug_delete + 1;
}
// Функция добавления в рендер предметы из файла
void pushThingFromFile(FILE* file, things* head) {
	things* element = getLast(head);
	things* tmp = (things*)malloc(sizeof(things));
	if (fscanf(file, "%d %d", &tmp->x, &tmp->y) != EOF) {
		tmp->next = NULL;
		element->next = tmp;
		// Увеличиваем счётчик кол-ва предметов
		bug_delete = bug_delete + 1;
	}
}


things* getPreLast (things* head) {
	if (head->next == NULL) {
		return NULL;
	}
	while (head->next->next) {
		head = head->next;
	}
	return head;
}

void deleteThing (things *head) {
	things* el = NULL;
	el = getPreLast(head);
	if (el == NULL) {
		free(head);
		head = NULL;
	} else {
		free(el->next);
		el->next = NULL;
	}
	// Уменьшаем счётчик кол-ва предметов
	bug_delete = bug_delete - 1;
}
// Переменные для считывания код цвета фона из файла
unsigned int ur, ug, ub;

int quit = 0;

// Структура для хранения информации о событии
SDL_Event event;

int main(int argc, char *argv[]) {
	FILE* save_file;
	SDL_Rect obj_size, screen_move;

	things* head = (things*)malloc(sizeof(things));
	head->x = 0;
	head->y = 0;
	head->next = NULL;

	// Считываем настройки игры из файла
	save_file = fopen("save_game.txt", "r");
	// Считываем размер окна
	fscanf(save_file, "%d %d", &SCREEN_WIDTH, &SCREEN_HEIGHT);
	// Считываем цвет фона
	fscanf(save_file, "%X %X %X", &ur, &ug, &ub);
	red = (uint8_t)ur;
	green = (uint8_t)ug;
	blue = (uint8_t)ub;
	// Считываем положение персонажа
	fscanf(save_file, "%d %d", &screen_move.x, &screen_move.y);
	// Считываем расположение предметов
	while (!feof(save_file)) {
		pushThingFromFile(save_file, head);
	}
	fclose(save_file);

	// Инициализируем библиотеку SDL
	if (initSDL() > 1) {
		printf("Error in initialization.\n");
	} else {
		// Загружаем картирнку из файла
		sprite_sheet = loadImage(sprite);
		wooden_sheet = loadImage(wooden);
		text_sheet = loadImage(textMenu);

		// Вызываем меню игры
		MenuMain(save_file);

		SDL_Rect wood_size, wood_move;
		// Растягиваем текстуру в полную картинку
		wood_size.x = 0;
		wood_size.y = 0;
		wood_size.h = 1500;
		wood_size.w = 1500;

		// Размер дерева
		wood_move.h = SCREEN_WIDTH / 7;
		wood_move.w = SCREEN_WIDTH / 7;

		last_frame = SDL_GetTicks();

		// Основной цикл программы, выход из которого происходит при
		// появлении события выхода (от ОС или по нажатию ESC)
		while (quit == 0) {
			// Ждём появления события
			while (SDL_PollEvent(&event)) {
				// При его появлении проверяем тип
				if (event.type == SDL_QUIT)
					// Если событие выхода (сигнал от ОС, что приложение
					// нужно завершить), меняем флаг выхода
					quit = 1;
				else {
					if (event.type == SDL_KEYDOWN) {
						// Если это нажатие на клавишу клавиатуры, смотрим код
						// нажатой клавиши
						switch (event.key.keysym.sym) {
						case SDLK_UP:	// Движение вверх
							if(screen_move.y >= -SCREEN_WIDTH / 40)
								screen_move.y -= SCREEN_WIDTH / 100;
							break;
						case SDLK_DOWN:	// Движение вниз
							if(screen_move.y <= SCREEN_HEIGHT - SCREEN_WIDTH / 10)
								screen_move.y += SCREEN_WIDTH / 100;
							break;
						case SDLK_LEFT:	// Движение влево
							if(screen_move.x >= -SCREEN_WIDTH / 30) {
								anim_type = 9;
								screen_move.x -= SCREEN_WIDTH / 100;
							}
							break;
						case SDLK_RIGHT:	// Движение вправо
							if(screen_move.x <= SCREEN_WIDTH - SCREEN_WIDTH / 15) {
								anim_type = 1;
								screen_move.x += SCREEN_WIDTH / 100;
							}
							break;
						case SDLK_a:	// Атака
							anim_phase = 0;
							// Удар в ту сторону, куда был направлен персонаж
							if(anim_type >= 0 && anim_type <= 7){
								anim_type = 4;
							} else {
								anim_type = 12;
							}
							break;
						case SDLK_f:	// Добавление предметов
							pushThing(head);
							break;
						case SDLK_d:	// Удаление предметов
							// Если нет предметов для удаления
							if(bug_delete != 0)
								deleteThing(head);
							break;
						case SDLK_r:
							red = red + 10;
							SDL_SetRenderDrawColor(renderer, red, green, blue, 0xFF);
							break;
						case SDLK_g:
							green = green + 10;
							SDL_SetRenderDrawColor(renderer, red, green, blue, 0xFF);
							break;
						case SDLK_b:
							blue = blue - 10;
							SDL_SetRenderDrawColor(renderer, red, green, blue, 0xFF);
							break;
						case SDLK_s:
							save_file = fopen("save_game.txt", "w");
							fprintf(save_file, "%d %d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
							fprintf(save_file, "%X %X %X\n", red, green, blue);
							fprintf(save_file, "%d %d\n", screen_move.x, screen_move.y);
							things* el = head->next;
							while (el) {
								fprintf(save_file, "%d %d\n", el->x, el->y);
								el = el->next;
							}
							fclose(save_file);
							break;
						case SDLK_ESCAPE:	// Выход из игры
							// Нажата клавиша ESC, меняем флаг выхода
							quit = 1;
							break;
						}
					} else {
						// Если персонаж был направлен вправо и нет анимации атаки, то проигрываем анимацию "на месте" вправо
						if(anim_type > 0 && anim_type < 8 && anim_type != 4)
							anim_type = 0;
						// Инче если персонаж был направлен влево и нет анимации атаки, то проигрываем анимацию "на месте" влево
						else if(anim_type > 7 && anim_type < 16 && anim_type != 12)
							anim_type = 8;
					}
				}
			}
			// отрисовка картинки с новым кадром анимации
			// проверка прошедшего времени:
			if ((SDL_GetTicks()- last_frame) >= frame_time) {
				anim_phase++;
				if (anim_phase >= anim_phase_max[anim_type]) {
					anim_phase = 0;
					if(anim_type == 4)
						anim_type = 0;
					else if(anim_type == 12)
						anim_type = 8;
				}
				last_frame = SDL_GetTicks();
			}

			// часть картинки с нужным кадром анимации
			obj_size.x = 32 * anim_phase;
			obj_size.y = 32 * anim_type;
			obj_size.h = 32;
			obj_size.w = 32;
			// место для вывода кадра анимации и его увеличение
			screen_move.h = SCREEN_WIDTH / 10;
			screen_move.w = SCREEN_WIDTH / 10;

			// Очищаем буфер рисования
			SDL_RenderClear(renderer);

			SDL_RenderCopy(renderer, sprite_sheet, &obj_size, &screen_move);

			things* last_el = head->next;
			// Добавление в список создаваемые предметы
			while (last_el) {
				wood_move.x = last_el->x;
				wood_move.y = last_el->y;
				last_el = last_el->next;
				// Добавление в рендер предметы
				SDL_RenderCopy(renderer, wooden_sheet, &wood_size, &wood_move);
			}
			// Выводим буфер на экран
			SDL_RenderPresent(renderer);

		}

	}
	closeSDL();
	return EXIT_SUCCESS;
}

// Функция второго экрана (выбора цвета)
void MenuColor(FILE* save_file) {
	// Читаем цвет из файла
	save_file = fopen("save_game.txt", "r");

	// Пропускаем одну стоку в файле (разрешение экрана)
	int val;
	val = fgetc(save_file);
	while(val != '\n'){
		val = fgetc(save_file);
	}

	fscanf(save_file, "%X %X %X", &ur, &ug, &ub);
	red = (uint8_t)ur;
	green = (uint8_t)ug;
	blue = (uint8_t)ub;
	fclose(save_file);
	SDL_SetRenderDrawColor(renderer, red, green, blue, 0xFF);

	int q = 0;
	while (quit == 0) {
		// Ждём появления события
		while (SDL_PollEvent(&event)) {
			SDL_Rect text_rend, text_size;
			text_size.w = SCREEN_WIDTH / 50; text_size.h = SCREEN_WIDTH / 50;
			SDL_RenderClear(renderer);
			// Ц
			text_rend.x = 640; text_rend.y = 730;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 2.12; text_size.y = SCREEN_HEIGHT / 3;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// В
			text_rend.x = 300; text_rend.y = 10;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 2.05; text_size.y = SCREEN_HEIGHT / 3;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// Е
			text_rend.x = 750; text_rend.y = 10;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 2; text_size.y = SCREEN_HEIGHT / 3;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// Т
			text_rend.x = 0; text_rend.y = 730;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 1.94; text_size.y = SCREEN_HEIGHT / 3;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// R
			text_rend.x = 820; text_rend.y = 1210;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 4; text_size.y = SCREEN_HEIGHT / 1.8;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// G
			text_rend.x = 0; text_rend.y = 1440;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 2; text_size.y = SCREEN_HEIGHT / 1.8;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// B
			text_rend.x = 170; text_rend.y = 1440;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 1.3; text_size.y = SCREEN_HEIGHT / 1.8;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// Д
			text_rend.x = 570; text_rend.y = 10;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 2.25; text_size.y = SCREEN_HEIGHT - SCREEN_WIDTH / 20;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// А
			text_rend.x = 0; text_rend.y = 10;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 2.16; text_size.y = SCREEN_HEIGHT - SCREEN_WIDTH / 20;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// Л
			text_rend.x = 850; text_rend.y = 250;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 2.09; text_size.y = SCREEN_HEIGHT - SCREEN_WIDTH / 20;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// Е
			text_rend.x = 770; text_rend.y = 10;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 2.04; text_size.y = SCREEN_HEIGHT - SCREEN_WIDTH / 20;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// Е
			text_rend.x = 770; text_rend.y = 10;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 2; text_size.y = SCREEN_HEIGHT - SCREEN_WIDTH / 20;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// (
			text_rend.x = 330; text_rend.y = 1440;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 1.95; text_size.y = SCREEN_HEIGHT - SCREEN_WIDTH / 20;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// Y
			text_rend.x = 530; text_rend.y = 1210;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 1.91; text_size.y = SCREEN_HEIGHT - SCREEN_WIDTH / 20;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			// )
			text_rend.x = 450; text_rend.y = 1440;
			text_rend.w = 170; text_rend.h = 170;
			text_size.x = SCREEN_WIDTH / 1.87; text_size.y = SCREEN_HEIGHT - SCREEN_WIDTH / 20;
			SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
			SDL_RenderPresent(renderer);

			// При его появлении проверяем тип
			if (event.type == SDL_QUIT)
				// Если событие выхода (сигнал от ОС, что приложение
				// нужно завершить), меняем флаг выхода
				quit = 1;
			else {
				if (event.type == SDL_KEYDOWN) {
					// Если это нажатие на клавишу клавиатуры, смотрим код
					// нажатой клавиши
					switch (event.key.keysym.sym) {
					case SDLK_r:
						red = red + 10;
						SDL_SetRenderDrawColor(renderer, red, green, blue, 0xFF);
						break;
					case SDLK_g:
						green = green + 10;
						SDL_SetRenderDrawColor(renderer, red, green, blue, 0xFF);
						break;
					case SDLK_b:
						blue = blue - 10;
						SDL_SetRenderDrawColor(renderer, red, green, blue, 0xFF);
						break;
					case SDLK_y:
						q = 1;
						break;
//					case SDL_BUTTON_LEFT:
//
//						break;
					case SDLK_ESCAPE:	// Выход из игры
						// Нажата клавиша ESC, меняем флаг выхода
						quit = 1;
						break;
					}
				}
			}
			if(q == 1) break;
		}
		if(q == 1) break;
	}
}

// Функция первого экрана меню с настройками
void MenuMain(FILE* save_file) {
	SDL_Rect text_rend, text_size;
	text_size.w = SCREEN_WIDTH / 50; text_size.h = SCREEN_WIDTH / 50;
	SDL_RenderClear(renderer);
	// И
	text_rend.x = 350; text_rend.y = 250;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.79; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// З
	text_rend.x = 200; text_rend.y = 250;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.68; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// М
	text_rend.x = 0; text_rend.y = 490;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.6; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Е
	text_rend.x = 760; text_rend.y = 10;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.49; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Н
	text_rend.x = 230; text_rend.y = 490;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.4; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// И
	text_rend.x = 350; text_rend.y = 250;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.34; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Т
	text_rend.x = 0; text_rend.y = 730;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.26; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Ь
	text_rend.x = 790; text_rend.y = 970;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.2; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Н
	text_rend.x = 230; text_rend.y = 490;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// А
	text_rend.x = 0; text_rend.y = 10;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.95; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// С
	text_rend.x = 870; text_rend.y = 490;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.9; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Т
	text_rend.x = 0; text_rend.y = 730;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.85; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Р
	text_rend.x = 730; text_rend.y = 490;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.81; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// О
	text_rend.x = 360; text_rend.y = 490;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.78; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Й
	text_rend.x = 500; text_rend.y = 250;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.73; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// К
	text_rend.x = 670; text_rend.y = 250;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.69; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// И
	text_rend.x = 350; text_rend.y = 250;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.65; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// ?
	text_rend.x = 350; text_rend.y = 1210;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.6; text_size.y = SCREEN_HEIGHT / 3;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Д
	text_rend.x = 570; text_rend.y = 10;
	text_rend.w = 170; text_rend.h = 190;
	text_size.x = SCREEN_WIDTH / 3.29; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// А
	text_rend.x = 0; text_rend.y = 10;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 3.10; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// (
	text_rend.x = 330; text_rend.y = 1440;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.98; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Y
	text_rend.x = 530; text_rend.y = 1210;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.92; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// )
	text_rend.x = 450; text_rend.y = 1440;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 2.85; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Н
	text_rend.x = 230; text_rend.y = 490;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.56; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Е
	text_rend.x = 760; text_rend.y = 10;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.535; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// Т
	text_rend.x = 0; text_rend.y = 730;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.505; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// (
	text_rend.x = 330; text_rend.y = 1440;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.48; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// N
	text_rend.x = 650; text_rend.y = 1210;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.47; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	// )
	text_rend.x = 450; text_rend.y = 1440;
	text_rend.w = 170; text_rend.h = 170;
	text_size.x = SCREEN_WIDTH / 1.435; text_size.y = SCREEN_HEIGHT / 1.8;
	SDL_RenderCopy(renderer, text_sheet, &text_rend, &text_size);
	SDL_RenderPresent(renderer);

	int q = 0;
	while (quit == 0) {
		// Ждём появления события
		while (SDL_PollEvent(&event)) {
			// При его появлении проверяем тип
			if (event.type == SDL_QUIT)
				// Если событие выхода (сигнал от ОС, что приложение
				// нужно завершить), меняем флаг выхода
				quit = 1;
			else {
				if (event.type == SDL_KEYDOWN) {
					// Если это нажатие на клавишу клавиатуры, смотрим код
					// нажатой клавиши
					switch (event.key.keysym.sym) {
					case SDLK_y:
						MenuColor(save_file);
						q = 1;
						break;
					case SDLK_n:
						q = 1;
						printf("#\n");
						break;
//					case SDL_BUTTON_LEFT:
//
//						break;
					case SDLK_ESCAPE:	// Выход из игры
						// Нажата клавиша ESC, меняем флаг выхода
						quit = 1;
						break;
					}
				}
			}
			if(q == 1) break;
		}
		if(q == 1) break;
	}
}

int initSDL() {
	int success = 1;
	// Инициализируем библиотеку SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		// при появлении ошибки выводим её описание
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		success = 0;
	} else {
		// После успешной инициализации создаём окно для вывода графики
		window = SDL_CreateWindow("My game", SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (window == NULL) {
			// выводим ошибку, если создать окно не удалось
			printf("Window could not be created! SDL_Error: %s\n",
					SDL_GetError());
			success = 0;
		} else {
			// Инициализируем библиотеку загрузки изображений (только для JPG)
			int imgFlags = IMG_INIT_JPG | IMG_INIT_PNG;
			if (!(IMG_Init(imgFlags) & imgFlags)) {
				printf("SDL_image could not initialize! SDL_image Error: %s\n",
				IMG_GetError());
				success = 0;
			}
			// Получаем поверхность для рисования
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (renderer == NULL) {
				// выводим ошибку, если создать окно не удалось
				printf("Renderer could not be created! SDL_Error: %s\n",
						SDL_GetError());
				success = 0;
			} else {
				// Задаём цвет отрисовки по умолчанию - белый
				SDL_SetRenderDrawColor(renderer, red, green, blue, 0xFF);
			}
		}
	}
	return success;
}

void closeSDL() {
	// Закрываем окно и освобождаем все выделенные ему ресурсы
	SDL_DestroyWindow(window);
	// Завершаем работу библиотеки и освобождаем все выделенные ей ресурсы
	SDL_Quit();
}

SDL_Texture* loadImage(char imagePath[]) {
	printf("%s\n", imagePath);
	SDL_Texture *newTexture;
	// Загружаем изображение
	SDL_Surface* loadedSurface = IMG_Load(imagePath);
	if (loadedSurface == NULL) {
		printf("Unable to load image %s! SDL_image Error: %s\n", imagePath,
		IMG_GetError());
	} else {
		// Если успешно загрузили, преобразуем загруженную поверхность в формат экранной
		// Если этого не сделать, то при каждом выводе будет проводится преобразование
		newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
		if (newTexture == NULL) {
			printf("Unable to load texture %s! SDL Error: %s\n", imagePath,
					SDL_GetError());
		} else
			// если успешно, возвращаем указатель на поверхность с изображенеим
			return newTexture;
		// Удаляем загруженную поверхность
		SDL_FreeSurface(loadedSurface);
	}
	return NULL;
}
