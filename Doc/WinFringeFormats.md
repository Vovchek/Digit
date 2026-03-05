.frn

1. `[ELLIPSES]`    
    Axis/center values are normalized to the range [0, 1] by dividing by the aperture max radius (see `[BOUNDS]`). 

    `ax by xc yc fi external unused`    
    `ax` - 1/2 axis along OX    
    `by` - 1/2 axis along OY    
    `xc`, `yc` - center coordinates    
    `fi` - angle of rotation in degrees    
    `external` - 1 if blinds outside shape, 0 - blinds inside shape
    `unused` -  seting to any value did not reveal any effect
2. `[BOUNDS]`    
   There may be up to 3 lines (with WinFringe) or more (manual addition) - aperture, externa scree, internal screen
    Each line has fields:
    `xl xr yd yu shape external`    
    `xl`, `xr` - left and right bounds along OX, image coordinates    
    `yd`, `yu` - down and up bounds along OY, image bottom->top coordinates    
    `shape` - 0 or 1 - ellipse, 2 - rectangle
    `external` - 1 if blinds outside shape, 0 - blinds inside shape
    1St line - aperture, other lines may be external or internal screen, depending on `external` field.