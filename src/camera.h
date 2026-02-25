#ifndef _CG_CAMERA
#define _CG_CAMERA


typedef struct Camera {
  CG_OffscreenBuffer *screenBuffer;
  float ppu;
}Camera;


Vec3 cam_world_to_screen_pos(Camera *_cam, Vec3 _pos);
Vec3 cam_world_to_screen_vec(Camera *_cam, Vec3 _vec);

#endif
