I came up with a new way to calculate AABB of an oriented box, that I thought 
would be faster than the existing one described in Real-Time Collision Detection.
But it turns out mine is orders of magnitude slower. 

I think it's mostly due to the ternary operations required in my method. 
