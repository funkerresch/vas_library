using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using MathHelpers;
using System;



[CustomEditor(typeof(VasSpatMaterialEq))]
public class VasSpatMaterialEqEditor : Editor
{
    enum DragOperation
    {
        Low,
        Mid,
        High,
    }

    private float masterGain;
    private float lowGain, midGain, highGain;
    private float lowFreq, midFreq, highFreq;
    private float midQ, lowQ, highQ;
    private bool useLogScale;
    private bool showSpectrum;

    DragOperation dragOperation = DragOperation.Low;

    public void DrawSpectrum(Rect r, bool useLogScale, float[] data, float dB_range, float samplerate, float col_r, float col_g, float col_b, float col_a, float gainOffset_dB)
    {
        float xscale = (float)(data.Length - 2) * 2.0f / samplerate;
        float yscale = 1.0f / dB_range;
        AudioCurveRendering.DrawCurve(
            r,
            delegate (float x)
            {
                double f = GUIHelpers.MapNormalizedFrequency((double)x, samplerate, useLogScale, true) * xscale;
                int i = (int)Math.Floor(f);
                double h = data[i] + (data[i + 1] - data[i]) * (f - i);
                double mag = (h > 0.0) ? (20.0f * Math.Log10(h) + gainOffset_dB) : -120.0;
                return (float)(yscale * mag);
            },
            new Color(col_r, col_g, col_b, col_a));
    }

    public void DrawBandSplitMarker(Rect r, float x, float w, bool highlight, Color color)
    {
        if (highlight)
            w *= 2.0f;

        EditorGUI.DrawRect(new Rect(r.x + x - w, r.y, 2 * w, r.height), color);
    }

    protected static Color ScaleAlpha(Color col, float blend)
    {
        return new Color(col.r, col.g, col.b, col.a * blend);
    }


    public void DrawFilterCurve(
        Rect r,
        float[] coeffs,
        bool lowGain, bool midGain, bool highGain,
        Color color,
        bool useLogScale,
        bool filled,
        double masterGain,
        double samplerate,
        double magScale)
    {
        double wm = -2.0f * 3.1415926 / samplerate;

        ComplexD one = new ComplexD(1.0f, 0.0f);
        AudioCurveRendering.AudioCurveEvaluator d = delegate (float x)
        {
            ComplexD w = ComplexD.Exp(wm * GUIHelpers.MapNormalizedFrequency((double)x, samplerate, useLogScale, true));
            ComplexD hl = one;
            ComplexD hp = one;
            ComplexD hh = one;
            ComplexD h = hh * hp * hl;
            double mag = masterGain + 10.0 * Math.Log10(h.Mag2());
            return (float)(mag * magScale);
        };

        if (filled)
            AudioCurveRendering.DrawFilledCurve(r, d, color);
        else
            AudioCurveRendering.DrawCurve(r, d, color);
    }


    public override void OnInspectorGUI()
    {
        //GUILayout.Label("This is a Label in a Custom Editor");
       //Rect r = GUILayoutUtility.GetRect(1, 10000, 100, 10000);
        GUILayoutOption[] textAreaOptions;
        int samplerate = 44100;
        textAreaOptions = new GUILayoutOption[]
         {
             GUILayout.Height(120),
             //add more layout options
         };
        Rect r = EditorGUILayout.GetControlRect(true, textAreaOptions);
        //GUI.BeginGroup(r);
        //AudioCurveRendering.DrawCurve(new Rect(0, 0, r.width, r.height), (t) => Mathf.Sin(t * Mathf.PI * 2), Color.black);
        //EditorGUI.DrawRect(r, Color.clear);
        //GUI.EndGroup();
        //EditorSceneManager.activeSceneChangedInEditMode += ChangedActiveScene;

        Event evt = Event.current;
        int controlID = GUIUtility.GetControlID(FocusType.Passive);
        EventType evtType = evt.GetTypeForControl(controlID);

        r = AudioCurveRendering.BeginCurveFrame(r);

        float thr = 4.0f;
        float x = evt.mousePosition.x - r.x;
        if (evtType == EventType.MouseDown && r.Contains(evt.mousePosition) && evt.button == 0)
        {
            float lf = (float)GUIHelpers.MapNormalizedFrequency(lowFreq, samplerate, useLogScale, false) * r.width;
            float mf = (float)GUIHelpers.MapNormalizedFrequency(midFreq, samplerate, useLogScale, false) * r.width;
            float hf = (float)GUIHelpers.MapNormalizedFrequency(highFreq, samplerate, useLogScale, false) * r.width;
            float ld = Mathf.Abs(x - lf);
            float md = Mathf.Abs(x - mf);
            float hd = Mathf.Abs(x - hf);
            float d = ld;
            dragOperation = DragOperation.Low;
            if (md < d)
            {
                d = md;
                dragOperation = DragOperation.Mid;
            }
            if (hd < d)
            {
                d = hd;
                dragOperation = DragOperation.High;
            }
            GUIUtility.hotControl = controlID;
            EditorGUIUtility.SetWantsMouseJumping(1);
            evt.Use();
        }
        else if (evtType == EventType.MouseDrag && GUIUtility.hotControl == controlID)
        {
            switch (dragOperation)
            {
                case DragOperation.Low:
                    lowFreq = Mathf.Clamp((float)GUIHelpers.MapNormalizedFrequency(GUIHelpers.MapNormalizedFrequency(lowFreq, samplerate, useLogScale, false) + evt.delta.x / r.width, samplerate, useLogScale, true), 10.0f, samplerate * 0.5f);
                    if (evt.shift)
                        lowQ = Mathf.Clamp(lowQ - evt.delta.y * 0.05f, 0.01f, 10.0f);
                    else
                        lowGain = Mathf.Clamp(lowGain - evt.delta.y * 0.5f, -100.0f, 100.0f);
                    break;
                case DragOperation.Mid:
                    midFreq = Mathf.Clamp((float)GUIHelpers.MapNormalizedFrequency(GUIHelpers.MapNormalizedFrequency(midFreq, samplerate, useLogScale, false) + evt.delta.x / r.width, samplerate, useLogScale, true), 10.0f, samplerate * 0.5f);
                    if (evt.shift)
                        midQ = Mathf.Clamp(midQ - evt.delta.y * 0.05f, 0.01f, 10.0f);
                    else
                        midGain = Mathf.Clamp(midGain - evt.delta.y * 0.5f, -100.0f, 100.0f);
                    break;
                case DragOperation.High:
                    highFreq = Mathf.Clamp((float)GUIHelpers.MapNormalizedFrequency(GUIHelpers.MapNormalizedFrequency(highFreq, samplerate, useLogScale, false) + evt.delta.x / r.width, samplerate, useLogScale, true), 10.0f, samplerate * 0.5f);
                    if (evt.shift)
                        highQ = Mathf.Clamp(highQ - evt.delta.y * 0.05f, 0.01f, 10.0f);
                    else
                        highGain = Mathf.Clamp(highGain - evt.delta.y * 0.5f, -100.0f, 100.0f);
                    break;
            }
            evt.Use();
        }
        else if (evtType == EventType.MouseUp && evt.button == 0 && GUIUtility.hotControl == controlID)
        {
            GUIUtility.hotControl = 0;
            EditorGUIUtility.SetWantsMouseJumping(0);
            evt.Use();
        }

        if (Event.current.type == EventType.Repaint)
        {
            //float blend = plugin.IsPluginEditableAndEnabled() ? 1.0f : 0.5f;
            float blend = 1.0f;

            // Mark bands (low, medium and high bands)
            DrawBandSplitMarker(r, (float)GUIHelpers.MapNormalizedFrequency(lowFreq, samplerate, useLogScale, false) * r.width, thr, GUIUtility.hotControl == controlID && dragOperation == DragOperation.Low, new Color(0, 0, 0, blend));
            DrawBandSplitMarker(r, (float)GUIHelpers.MapNormalizedFrequency(midFreq, samplerate, useLogScale, false) * r.width, thr, GUIUtility.hotControl == controlID && dragOperation == DragOperation.Mid, new Color(0.5f, 0.5f, 0.5f, blend));
            DrawBandSplitMarker(r, (float)GUIHelpers.MapNormalizedFrequency(highFreq, samplerate, useLogScale, false) * r.width, thr, GUIUtility.hotControl == controlID && dragOperation == DragOperation.High, new Color(1.0f, 1.0f, 1.0f, blend));

            const float dbRange = 40.0f;
            const float magScale = 1.0f / dbRange;

            //float[] coeffs;
            //plugin.GetFloatBuffer("Coeffs", out coeffs, 15);

            // Draw filled curve
            DrawFilterCurve(
                r,
                null,
                true, true, true,
                ScaleAlpha(AudioCurveRendering.kAudioOrange, blend),
                useLogScale,
                false,
                masterGain,
                samplerate,
                magScale);

            if (GUIUtility.hotControl == controlID)
                DrawFilterCurve(
                    r,
                    null,
                    dragOperation == DragOperation.Low,
                    dragOperation == DragOperation.Mid,
                    dragOperation == DragOperation.High,
                    new Color(1.0f, 1.0f, 1.0f, 0.2f * blend),
                    useLogScale,
                    true,
                    masterGain,
                    samplerate,
                    magScale);

            //if (showSpectrum)
            //{
            //    int specLen = (int)r.width;
            //    float[] spec;

            //    //plugin.GetFloatBuffer("InputSpec", out spec, specLen);
            //    DrawSpectrum(r, useLogScale, spec, dbRange, samplerate, 0.3f, 1.0f, 0.3f, 0.5f * blend, 0.0f);

            //    //plugin.GetFloatBuffer("OutputSpec", out spec, specLen);
            //    DrawSpectrum(r, useLogScale, spec, dbRange, samplerate, 1.0f, 0.3f, 0.3f, 0.5f * blend, 0.0f);
            //}

            GUIHelpers.DrawFrequencyTickMarks(r, samplerate, useLogScale, new Color(1.0f, 1.0f, 1.0f, 0.3f * blend));
        }

        AudioCurveRendering.EndCurveFrame();
        
    }


}