#include "boardloader.h"

void load_board(Board *board, PelletHolder *pelletHolder, const char* file)
{
	FILE *fp = fopen(file, "r");

	if (fp == NULL)
	{
		printf("File %s cannot be found.\n", file);
		printf("Aborting.\n");
		exit(1);
	}

	char c = 'a';
	int x = 0;
	int y = 0;

	int pelletI = 0;

	bool walkable = false;

	while (((c = fgetc(fp)) != EOF))
	{
		if (c == '\n' || c == 13) continue; //continue if a newline or carriage-return

		//	NOTE:
		//
		// Some of these characters look identical (═ and =, for example).
		// They are different, and used to distinguish between different orientations of the same tile on the board.
		// Be wary of this when editing this code.
		
		SDL_Surface *image = NULL;

		switch (c)
		{
			case '0': image = double_corner_image(BOTTOM_LEFT);		break;
			case '1': image = double_corner_image(BOTTOM_RIGHT); 	break;
			case '2': image = double_corner_image(TOP_LEFT); 		break;
			case '3': image = double_corner_image(TOP_RIGHT); 		break;

			case '4': image = single_corner_image(BOTTOM_LEFT);		break;
			case '5': image = single_corner_image(BOTTOM_RIGHT); 	break;
			case '6': image = single_corner_image(TOP_LEFT); 		break;
			case '7': image = single_corner_image(TOP_RIGHT); 		break;
			
			case '8': image = tleft_image(TOP_RIGHT);				break;
			case '9': image = tleft_image(TOP_LEFT); 				break;
			case 'a': image = tleft_image(BOTTOM_LEFT); 			break;
			case 'b': image = tleft_image(BOTTOM_RIGHT); 			break;
			
			case 'c': image = tright_image(TOP_LEFT);				break;
			case 'd': image = tright_image(BOTTOM_RIGHT); 			break;
			case 'e': image = tright_image(BOTTOM_LEFT); 			break;
			case 'f': image = tright_image(TOP_RIGHT); 				break;
			
			case 'g': image = hallway_image(UP);					break;
			case 'h': image = hallway_image(DOWN); 					break;
			case 'i': image = hallway_image(LEFT); 					break;
			case 'j': image = hallway_image(RIGHT); 				break;
			
			case 'k': image = middle_image(LEFT); 					break;
			case 'l': image = middle_image(RIGHT); 					break;
			case 'm': image = middle_image(UP); 					break;
			case 'n': image = middle_image(DOWN);	 				break;

			case 'o': image = pen_corner_image(TOP_LEFT);			break;
			case 'p': image = pen_corner_image(TOP_RIGHT); 			break;
			case 'q': image = pen_corner_image(BOTTOM_LEFT); 		break;
			case 'r': image = pen_corner_image(BOTTOM_RIGHT); 		break;

			case 's': image = pen_side_image(UP); 					break;
			case 't': image = pen_side_image(DOWN); 				break;
			case 'u': image = pen_side_image(LEFT); 				break;
			case 'v': image = pen_side_image(RIGHT);	 			break;

			case 'w': image = pen_gate_image();						break;

			case '.': /* pellet */ 	
				pelletHolder->pellets[pelletI].image = small_pellet_image();
				pelletHolder->pellets[pelletI].x = x;
				pelletHolder->pellets[pelletI].y = y;
				pelletHolder->pellets[pelletI].eaten = false;
				pelletHolder->pellets[pelletI].type = SmallPellet;				
				pelletI++;

				walkable = true;									
				break;
			case '*': /* large pellet */
				pelletHolder->pellets[pelletI].image = large_pellet_image();
				pelletHolder->pellets[pelletI].x = x;
				pelletHolder->pellets[pelletI].y = y;
				pelletHolder->pellets[pelletI].eaten = false;
				pelletHolder->pellets[pelletI].type = LargePellet;				
				pelletI++;

				walkable = true;									
				break;
			case ',': /* empty, walkable square */ 					
				walkable = true;									
				break;
			case ' ': /* empty, nonwalkable square */ 				break; 
			default: 
			printf("Error loading character: %c (int: %d) at coordinate (%d, %d) in boardfile %s\n", c, c, x, y, file);
			printf("aborting\n");
			exit(1);
			break;
		}

		board->boardSquares[x][y].image = image;
		board->boardSquares[x][y].walkable = walkable;

		image = NULL;
		walkable = false;
		x++;

		if (x == BOARD_LENGTH)
		{
			x = 0;
			y++;
		}
	}

	fclose(fp);
}
