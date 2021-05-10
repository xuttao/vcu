AREA |.text|, CODE, READONLY      ; name this block of code
        EXPORT ImgYUV2RGB24_neon
		
		ALIGN
;void ImgYUV2RGB24_neon(u8 *pu8RgbBuffer, u8 *pu8SrcYUV, l32 l32Width, l32 l32Height)
ImgYUV2RGB24_neon
    ;push            {r4, r5, r6, r7, r8, r9, r10, lr}
    stmfd			sp!, {r4-r10,lr}
    
    add             r4,  r2, r2            
    add             r4,  r4, r2             ;r4 : DstStride = 3 * l32Width
    
    mul             r5, r4, r3
    sub             r5, r5, r4
    add             r0, r0, r5             ;r0: pu8Dst = pu8Dst + l32DstStride * (l32Height - 1)
    
    mul             r5, r2, r3             
    add             r6, r1, r5             ;r6 : pu8SrcU = pu8SrcYUV + l32Width * l32Height
    add             r7, r6, r5, lsr #2     ;r7 : pu8SrcV = pu8SrcU + ((l32Width * l32Height)>>2)
    
    ;lsr             r8, r2, #3             ;r8 记录了col的循环次数， r2记录了YUV图像宽度
    mov				 r8, r2, lsr #3
    ;lsr             lr, r3, #1             ;lr 记录了Row的循环次数， r3记录了YUV图像高度
    mov				 lr, r3, lsr #1
    
    add             r3, r1, r2             ;r1, pu8Src1;  r3 : pu8Src2, r2 : l32Width
    sub             r5, r0, r4             ;r5 : pu8Dst2 = pu8Dst - l32DstStride
    
    mov             r9, #16
	vdup.8          d8, r9
	
	mov             r10, #128
	vdup.8          d9, r10
	
	mov             r9, #75
	vdup.16         q5, r9 				;q5: 75
	
	mov             r10, #102
	vdup.16         q6, r10				;q6: 102
	
	mov             r9, #25
	vdup.16         q7, r9 				;q7: 25
	
	mov             r10, #52
	vdup.16         q8, r10				;q8: 52
	
	mov             r9, #129
	vdup.16         q9, r9				;q9: 129
        
loop_row
loop_col
	subs            r8, r8, #1
	vld1.u8         d0, [r1]!     		;YLine1
	vld1.u8         d2, [r3]!     		;YLine2
	vld1.32         {d4[0]}, [r6]!      ;U
	vld1.32         {d4[1]}, [r7]!		;V
	
	vsubl.u8        q0, d0, d8         ;YLine2 - 16
	vsubl.u8        q1, d2, d8         ;YLine1 - 16	
	
	vsubl.u8        q2, d4, d9
	vmov            q3, q2
	vzip.s16		q2, q3               ;q2:U - 128  q3: V-128
	
	;开始计算乘法部分				
	vmul.s16         q10, q3, q8			
	vmla.s16         q10, q2, q7	 ;得到计算G分量所需要的后半部分U、V之和
	
	vmul.s16         q11, q2, q9	 ;得到计算B分量的后半部分所需要的U
	
	vmul.s16         q12, q3, q6	 ;得到计算R分量的后半部分所需要的V	
	
	;计算Y的部分乘积
	vmul.s16       	q0, q0, q5	     ;q0、q1得到第一行Y的共8点乘积
	vmul.s16        q1, q1, q5	     ;q2、q3得到第二行Y的共8点乘积
	
	;得到两行的G分量
	vqsub.s16        q13, q0, q10
	vqsub.s16        q14, q1, q10
	
	vqrshrun.s16     d27, q13, #6  		;;;;;;;;;;;;;;;;;;第一行的G   
	vqrshrun.s16     d30, q14, #6		;;;;;;;;;;;;;;;;;;第二行的G	
	
	;得到两行的B分量
	vqadd.s16       q10, q0, q11
	vqadd.s16       q11, q1, q11
	
	vqrshrun.s16    d26, q10, #6			;;;;;;;;;;;;;;;;;;第一行的B
	vqrshrun.s16    d29, q11, #6			;;;;;;;;;;;;;;;;;;第二行的B
	
	;得到两行的R分量	
	vqadd.s16       q11, q0, q12
	vqadd.s16       q12, q1, q12
	
	vqrshrun.s16    d28, q11, #6			;;;;;;;;;;;;;;;;;;第一行的R
	vqrshrun.s16    d31, q12, #6			;;;;;;;;;;;;;;;;;;第二行的R
	
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;做interleave操作，形成RGB形式，然后存入目标buffer
	vst3.8          {d26, d27, d28}, [r0]!
	
	vst3.8          {d29, d30, d31}, [r5]!
	
	bgt             loop_col
	
	subs            lr, lr, #1 
	
	sub             r0, r5, r4, lsl #1
	sub             r5, r0, r4
	
	add             r1, r1, r2
	add             r3, r3, r2
	;lsr             r8, r2, #3  
	mov				r8, r2, lsr #3
    bgt             loop_row
 
    ;pop             {r4, r5, r6, r7, r8, r9, r10, lr}
    ldmfd			sp!, {r4-r10,lr}
    bx              lr
		
    END
