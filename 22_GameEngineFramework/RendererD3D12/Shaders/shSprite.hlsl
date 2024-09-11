Texture2D texDiffuse : register(t0);
SamplerState samplerDiffuse : register(s0);

cbuffer CONSTANT_BUFFER_SPRITE : register(b0)
{
    float2 g_ScreenRes;
    float2 g_Pos;
    float2 g_Scale;
    float2 g_TexSize;
    float2 g_TexSampePos;
    float2 g_TexSampleSize;
    float g_Z;
    float g_Alpha;
    float g_Reserved0;
    float g_Reserved1;

};

struct VSInput
{
    float4 Pos : POSITION;
    float4 color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float2 TexCoord : TEXCOORD0;
};

PSInput VSMain(VSInput input)
{
    PSInput result = (PSInput)0;
    
    float2 scale = (g_TexSize / g_ScreenRes) * g_Scale;
    float2 offset = (g_Pos / g_ScreenRes); // float좌표계 기준 지정한 위치
    float2 Pos = input.Pos.xy * scale + offset;
    result.position = float4(Pos.xy * float2(2, -2) + float2(-1, 1), g_Z, 1); // 정규좌표게로 변환
 
    float2 tex_scale = (g_TexSampleSize / g_TexSize);
    float2 tex_offset = (g_TexSampePos / g_TexSize);
    result.TexCoord = input.TexCoord * tex_scale + tex_offset;
    //result.TexCoord = mul(float4(input.TexCoord, 0, 1), g_matTransformUV);
    
    result.color = input.color;
    
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 texColor = texDiffuse.Sample(samplerDiffuse, input.TexCoord);
    return texColor * input.color;
}
