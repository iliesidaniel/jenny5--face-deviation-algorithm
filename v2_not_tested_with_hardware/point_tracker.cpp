#include "point_tracker.h"


tracking_data get_offset_angles(int webcam_model_number, float image_ratio, int image_width, int image_height, Point position)
{
	tracking_data deviation;

	if (webcam_model_number == 910)
	{
		if (image_ratio == 4 / 3)
		{
			deviation.grades_from_center_x = determine_offset_angle(position.x, C910_4_3_HORIZONTAL_FIELD_OF_VIEW, image_width);
			deviation.grades_from_center_y = determine_offset_angle(position.y, C910_4_3_VERTICAL_FIELD_OF_VIEW, image_height);
		} else {
			deviation.grades_from_center_x = determine_offset_angle(position.x, C910_16_9_HORIZONTAL_FIELD_OF_VIEW, image_width);
			deviation.grades_from_center_y = determine_offset_angle(position.y, C910_16_9_VERTICAL_FIELD_OF_VIEW, image_height);
		}
	} 
	else if (webcam_model_number == 920)
	{
		if (image_ratio == 4 / 3)
		{
			deviation.grades_from_center_x = determine_offset_angle(position.x, C920_4_3_HORIZONTAL_FIELD_OF_VIEW, image_width);
			deviation.grades_from_center_y = determine_offset_angle(position.y, C920_4_3_VERTICAL_FIELD_OF_VIEW, image_height);
		} else {
			deviation.grades_from_center_x = determine_offset_angle(position.x, C920_16_9_HORIZONTAL_FIELD_OF_VIEW, image_width);
			deviation.grades_from_center_y = determine_offset_angle(position.y, C920_16_9_VERTICAL_FIELD_OF_VIEW, image_height);
		}
	}

	return deviation;
}

float determine_offset_angle(int position, float field_of_view, int number_of_pixels)
{
	float pixel_one_percent = (float)number_of_pixels / 100;
	float fov_one_percent = (float)field_of_view / 100;
	float offset_from_center = (position - number_of_pixels / 2) / pixel_one_percent; 
	
	return offset_from_center * fov_one_percent;
}