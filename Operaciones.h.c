#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

const Minimo = 0x80000000;
const Maximo = 0xFFFFFFFF;
int Get_Valor(Vmx vmx , int tipo , int dato) {
    int valor = 0 , direccion;
    if (tipo != 0) {
      LeerCodeSegment(*vmx , &direccion , tipo); // lee el codigo y los guarda en direccion
      if (tipo == 1)
        valor = vmx->VRegistro[direccion];
      else
        if (tipo == 2)
          valor = vmx->Memoria[direccion];
        else
            valor = direccion;
    }
    return valor;
}

int Validar_Operacion(int CodOP ,int tipoA ,int tipoB) {
    int val = 0;
    if (tipoA >= 0 && tipoA <= 3)
      if (tipoB >= 0 && tipoB <= 3)
         if (CodOP >= 0 && CodOP <= 31)
            if (CodOP <= 10  || CodOP >= 15)
               val = 1;
    return val;
}

void Realizar_Operacion(Vmx *vmx , VOperacion vo ,int CodOP ,int tipoA ,int tipoB) {
     if (!Validar_Operacion(CodOP , tipoA , tipoB) )
        Detener_Ejecusion();
    else
        if (CodOP <= 10)
            vo[CodOP](vmx , tipoA,Get_Dato(*vmx,tipoA));
        else
           if (CodOP == 15)
                vo[CodOP](vmx );
            else
                vo[CodOP](vmx,tipoA, Get_Dato(*vmx,tipoA), tipoB,Get_Dato(*vmx,tipoB));
}


void SYS(Vmx *vmx , int tipoA ,int datoA) {
    int32_t valorA = Get_Valor(vmx, tipoA, datoA);
    int i , n = vmx->VRegistro[12] & 0xFFFF0000 ;
    int limite = vmx->VRegistro[12] & 0x0000FFFF;
    char v[31];
            if (valorA == 1) {
                while (i < n) {
                    scanf("%s",v);
                    

                }
            }
            else 
                if (dato == 2)
                    for (i = 0 ; i < n ; i ) {
                        aux = vmx->Memoria[vmx->VRegistro[13]];
                        aux =
                    }
                else
                    Detener_Ejecusion();
                
}

void realizar_salto(Vmx *vmx , int32_t Salto) {
    vmx->VRegistro[0] = Salto;
}

void JPM(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
        realizar_salto(vmx , valorA);
}

void JP(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
        if ((vmx->VRegistro[17] & 16) == 0 && (vmx->VRegistro[17] & 8) == 0)
            realizar_salto(vmx , valorA);
}

void JN(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
        if (vmx->VRegistro[17] & 16 && (vmx->VRegistro[17] & 8) == 0)
            realizar_salto(vmx , valorA);
}

void JZ(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
        if ((vmx->VRegistro[17] & 16) == 0 && vmx->VRegistro[17] & 8)
            realizar_salto(vmx , valorA);
}

void JC(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
        if (vmx->VRegistro[17] & 2)
            realizar_salto(vmx , valorA);
}

void JV(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
        if (vmx->VRegistro[17] & 1)
            realizar_salto(vmx , valorA);
}

void JNP(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
        if (vmx->VRegistro[17] & 16 || vmx->VRegistro[17] & 8 )
            realizar_salto(vmx , valorA);
}

void JNN(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
        if ((vmx->VRegistro[17] & 16) == 0 )
            realizar_salto(vmx , valorA);
}

void JNZ(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
        if ((vmx->VRegistro[17] & 8) == 0  )
            realizar_salto(vmx , valorA);
}

void NOT(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t Resultado = ~valorA;
    int carry = 0 , desbordamiento = 0;
        ActualizarCC(vmx , Resultado , carry , desbordamiento);
        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void STOP(Vmx *vmx) {
    vmx->VRegistro[0] = -1;
}

void MOV(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int carry = 0 , desbordamiento = 0;
      ActualizarCC(vmx , valorB , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , valorB);
}


void ADD(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA + valorB ;
    int carry = ((uint64_t)(uint32_t)valorA + (uint32_t)valorB) > Maximo;
    int desbordamiento = (valorA > 0 && valorB > 0 && Resultado < 0) || (valorA < 0 && valorB < 0 && Resultado >= 0);
      ActualizarCC(vmx , Resultado , carry , desbordamiento);
      Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}


void SUB(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA - valorB ;
    int carry = (uint32_t)valorA < (uint32_t)valorB;
    int desbordamiento = (valorA >= 0 && valorB < 0 && Resultado < 0) || (valorA < 0 && valorB > 0 && Resultado >= 0);
      ActualizarCC(vmx , Resultado , carry , desbordamiento);
      Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void MUL(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA * valorB;
    int carry = ((uint64_t)(uint32_t)valorA * (uint32_t)valorB) >  Maximo;
    int desbordamiento = ((int64_t)valorA * valorB > Maximo || (int64_t)valorA * valorB < Minimo);
      ActualizarCC(vmx , Resultado , carry , desbordamiento);
       Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void DIV(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA  = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado ;
    int carry = 0;
    int desbordamiento = valorA == Minimo && valorB == -1;
        if (valorB == 0)
            Detener_Ejecusion();
        Resultado = valorA / valorB ;
        vmx->VRegistro[16] = valorA % valorB;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void CMP(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA - valorB;
    int carry = (uint32_t)valorA < (uint32_t)valorB;
    int desbordamiento = ((valorA >= 0 && valorB < 0 && Resultado < 0) || (valorA < 0 && valorB > 0 && Resultado >= 0));
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

}

void AND(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA & valorB;
    int carry = 0 , desbordamiento = 0;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void OR(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA | valorB ;
    int carry = 0 , desbordamiento = 0;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void XOR(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA ^ valorB ;
    int carry = 0 , desbordamiento = 0;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void SWAP(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = valorB;
    int32_t aux = valorA;
    int carry = 0 , desbordamiento = 0;
        ActualizarCC(vmx , Resultado , carry , desbordamiento);
        Guardar_Resultado(vmx , tipoA , datoA , valorB);
        Guardar_Resultado(vmx , tipoB , datoB , aux);
}

void SHL(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado ;
    int32_t carry = 0;
    int desbordamiento = ((int64_t)valorA * ((int64_t)1 << valorB) > Maximo || (int64_t)valorA * ((int64_t)1 << valorB) < Minimo);
    int i = 0;
        while (i < valorB && carry == 0) {
            if ((valorA >> (31 - i)) & 1)
                carry = 1;
                i++;
        }
        Resultado = valorA << valorB ;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void SHR(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = 0;
    int  i , aux , n = 32 - valorB;
    int carry = 0 , desbordamiento = 0;
        i = 0;
        while (i < valorB && carry == 0) {
            if ((valorA >> (31 - i)) & 1)
                carry = 1;
                i++;
        }
        valorA = valorA >> valorB;
        for (i = 0 ; i < n ; i ++ ) {
            aux = valorA & 1;
            aux = aux << i;
            Resultado += aux;
            valorA = valorA >> 1;
        }
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void SAR(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = 0;
    int  carry = 0 , desbordamiento = 0 , i;
        i = 0;
        while (i < valorB && carry == 0) {
            if ((valorA >> (31 - i)) & 1)
                carry = 1;
                i++;
        }
        Resultado = valorA >> valorB ;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void LDL(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = 0;
    int  carry = 0 , desbordamiento = 0;
        Resultado = valorA >> valorB ;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void LDH(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = 0;
    int  carry = 0 , desbordamiento = 0;
        Resultado = valorA >> valorB ;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void RND(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = Get_Valor(vmx, tipoA, datoA);
    int32_t  valorB = Get_Valor(vmx, tipoB, datoB);
    int32_t Resultado = rand() % (valorB + 1);;
    int  carry = 0 , desbordamiento = 0;
        Resultado = valorA >> valorB ;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);
      Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}
