I came up with a new way to calculate AABB of a oriented box, that I thought 
would be faster than the existing one described in Real-Time Collision Detection.
But it turns down mine is significantly slower. 

I think it's mostly due to the ternary operations required in my method. 
