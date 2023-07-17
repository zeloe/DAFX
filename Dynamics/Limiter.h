/*
  ==============================================================================

    Limiter.h
    Created: 17 Jul 2023 8:14:48pm
    Author:  Onez

  ==============================================================================
*/

#pragma once
class Limiter
{
public:
    Limiter(){}
    ~Limiter(){}

    void prepare(int blocksize) noexcept
    {
        bs = blocksize;
        attack = 0;
        release = 0;
        del4 = 0;
        del3 = 0;
        del2 = 0;
        del1 = 0;
        
    }
    
    void setParams(float at, float rt, float th)
    {
        attack = at;
        release = rt;
        tresh = th;
    }
    
    void process(float* input)
    {
        if(attack != current_attack || release != current_release || tresh != current_tresh)
        {
            inc_attack = (attack - current_attack) / bs;
            inc_release = (release - current_release) / bs;
            inc_tresh = (tresh - current_tresh) / bs;
            for(int i = 0; i < bs; i++)
            {
                current_attack += inc_attack;
                current_release += inc_release;
                current_tresh += inc_tresh;
                float a = fabs(del4);
                if(a > 0)
                {
                    coeff = current_attack;
                }
                else
                {
                    coeff = current_release;
                }
                
                xpeak =(1 - coeff) * xpeak + coeff * a;
                xpeak = current_tresh / xpeak;
                if(xpeak > 1.0)
                {
                    f = 1.0;
                }
                else
                {
                    f = xpeak;
                }
                if(f > g)
                {
                    coeff = current_attack;
                }
                else
                {
                    coeff = current_release;
                }
                
                g = (1.0 - coeff) * g + coeff * f;
                
                del4 = del3  * g;
                del3 = del2  * g;
                del2 = del1  * g;
                del1 = input[i]  * g;
    
                
                input[i] = del4 * g;
                
                
                
                
            }
            current_attack = attack;
            current_release = release;
            current_tresh = tresh;
        }
        else
        {
            for(int i = 0; i < bs; i++)
            {
                float a = fabs(del4);
                if(a > 0)
                {
                    coeff = current_attack;
                }
                else
                {
                    coeff = current_release;
                }
                
                xpeak =(1 - coeff) * xpeak + coeff * a;
                //here is something wrong with fmin
                xpeak = current_tresh / xpeak;
                if(xpeak > 1.0)
                {
                    f = 1.0;
                }
                else
                {
                    f = xpeak;
                }
                if(f > g)
                {
                    coeff = current_attack;
                }
                else
                {
                    coeff = current_release;
                }
                
                g = (1.0 - coeff) * g + coeff * f;
                
                del4 = del3  * g;
                del3 = del2  * g;
                del2 = del1  * g;
                del1 = input[i]  * g;
                
                
                input[i] = del4 * g;
            }
        }
        
        
        
    }
    
    
private:
    float attack;
    float release;
    float tresh;
    
    float current_attack;
    float current_release;
    float current_tresh;
    
    float inc_attack;
    float inc_release;
    float inc_tresh;
    
    float bs;
    float del1;
    float del2;
    float del3;
    float del4;
    float coeff;
    float xpeak;
    float g;
    float f;
    
};
