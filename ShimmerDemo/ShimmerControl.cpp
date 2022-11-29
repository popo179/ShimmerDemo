//
// ShimmerControl.cpp
// Implementation of the ShimmerControl class.
//

#include "pch.h"
#include "ShimmerControl.h"
#include <WindowsNumerics.h>


using namespace ShimmerDemo;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Documents;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::Foundation::Numerics;

// The Templated Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234235

DependencyProperty^ ShimmerControl::s_animationDurationInMillisecondsProperty = nullptr;
DependencyProperty^ ShimmerControl::s_isAnimatingProperty = nullptr;

constexpr float InitialStartPointX = -7.92f;

ShimmerControl::ShimmerControl()
{
	DefaultStyleKey = "ShimmerDemo.ShimmerControl";
    Loaded += ref new RoutedEventHandler(this, &ShimmerControl::onLoaded);
}

DependencyProperty^ ShimmerControl::AnimationDurationInMillisecondsProperty::get()
{
    return s_animationDurationInMillisecondsProperty;
}

DependencyProperty^ ShimmerControl::IsAnimatingProperty::get()
{
    return s_isAnimatingProperty;
}

double ShimmerControl::AnimationDurationInMilliseconds::get()
{
    return static_cast<double>(GetValue(AnimationDurationInMillisecondsProperty));
}

void ShimmerControl::AnimationDurationInMilliseconds::set(double value)
{
    SetValue(AnimationDurationInMillisecondsProperty, value);
}

bool ShimmerControl::IsAnimating::get()
{
    return static_cast<bool>(GetValue(IsAnimatingProperty));
}

void ShimmerControl::IsAnimating::set(bool value)
{
    SetValue(IsAnimatingProperty, value);
}


void ShimmerControl::RegisterDependencyProperties()
{
    s_animationDurationInMillisecondsProperty = DependencyProperty::Register(
        "AnimationDurationInMilliseconds",
        double::typeid,
        ShimmerControl::typeid,
        ref new PropertyMetadata(
            1600.0,
            ref new PropertyChangedCallback(&ShimmerControl::OnAnimationDurationInMillisecondsPropertyChanged)));

    s_isAnimatingProperty = DependencyProperty::Register(
        "IsAnimating",
        bool::typeid,
        ShimmerControl::typeid,
        ref new PropertyMetadata(
            true,
            ref new PropertyChangedCallback(&ShimmerControl::OnIsAnimatingPropertyChanged)));
}

void ShimmerControl::OnAnimationDurationInMillisecondsPropertyChanged(DependencyObject^ d, DependencyPropertyChangedEventArgs^ e)
{
    ShimmerControl^ control = static_cast<ShimmerControl^>(d);
    if (control->IsAnimating)
    {
        control->TryStartAnimation();
    }
}


void ShimmerControl::OnIsAnimatingPropertyChanged(
    DependencyObject^ d,
    DependencyPropertyChangedEventArgs^ e)
{
    ShimmerControl^ control = static_cast<ShimmerControl^>(d);
    auto isAnimating = static_cast<bool>(e->NewValue);
    if (isAnimating)
    {
        control->TryStartAnimation();

    }
    else
    {
        control->StopAnimation();
    }
}

void ShimmerControl::onLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if (!m_initialized && TryInitializationResource() && IsAnimating)
    {
        TryStartAnimation();
    }
    ActualThemeChanged += ref new TypedEventHandler<FrameworkElement^, Object^>(this, &ShimmerControl::OnActualThemeChanged, CallbackContext::Same);
}

void ShimmerControl::OnActualThemeChanged(FrameworkElement^ /*sender*/, Object^ /*args*/)
{
    if (!m_initialized)
    {
        return;
    }
    SetGradientStopColorsByTheme();
}
void ShimmerControl::TryStartAnimation()
{
    if (m_isAnimationStarted || m_initialized == false || m_shape == nullptr)
    {
        return;
    }
    auto hostVisual = Windows::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(m_shape);
    auto sizeAnimation = hostVisual->Compositor->CreateExpressionAnimation("hostVisual.Size");
    sizeAnimation->SetReferenceParameter("hostVisual", hostVisual);
    m_shapeVisual->StartAnimation(L"Size", sizeAnimation);
    m_rectangleGeometry->StartAnimation(L"Size", sizeAnimation);

    auto gradientStartPointAnimation = hostVisual->Compositor->CreateVector2KeyFrameAnimation();
    gradientStartPointAnimation->Duration = TimeSpanFromMilliseconds(AnimationDurationInMilliseconds);
    gradientStartPointAnimation->IterationBehavior = Windows::UI::Composition::AnimationIterationBehavior::Forever;
    gradientStartPointAnimation->InsertKeyFrame(0.0f, float2(InitialStartPointX, 0.0f));
    gradientStartPointAnimation->InsertKeyFrame(1.0f, float2(0.0f, 0.0f));
    m_shimmerMaskGradient->StartAnimation(L"StartPoint", gradientStartPointAnimation);

    auto gradientEndPointAnimation = hostVisual->Compositor->CreateVector2KeyFrameAnimation();
    gradientEndPointAnimation->Duration = TimeSpanFromMilliseconds(AnimationDurationInMilliseconds);
    gradientEndPointAnimation->IterationBehavior = Windows::UI::Composition::AnimationIterationBehavior::Forever;
    gradientEndPointAnimation->InsertKeyFrame(0.0f, float2(1.0f, 0.0f)); //Vector2.One
    gradientEndPointAnimation->InsertKeyFrame(1.0f, float2(-InitialStartPointX, 1.0f));
    m_shimmerMaskGradient->StartAnimation(L"EndPoint", gradientEndPointAnimation);

    m_isAnimationStarted = true;

}

void ShimmerControl::StopAnimation()
{
    if (!m_isAnimationStarted)
    {
        return;
    }
    m_shimmerMaskGradient->StopAnimation(L"StartPoint");
    m_shimmerMaskGradient->StopAnimation(L"EndPoint");
}

bool ShimmerControl::TryInitializationResource()
{
    if (m_initialized)
    {
        return true;
    }
    if (m_shape == nullptr || IsLoaded == false)
    {
        return false;
    }
    auto hostVisual = Windows::UI::Xaml::Hosting::ElementCompositionPreview::GetElementVisual(m_shape);

    m_compositor = hostVisual->Compositor;
    m_rectangleGeometry = m_compositor->CreateRoundedRectangleGeometry();
    m_shapeVisual = m_compositor->CreateShapeVisual();
    m_shimmerMaskGradient = m_compositor->CreateLinearGradientBrush();
    m_gradientStop1 = m_compositor->CreateColorGradientStop();
    m_gradientStop2 = m_compositor->CreateColorGradientStop();
    m_gradientStop3 = m_compositor->CreateColorGradientStop();
    m_gradientStop4 = m_compositor->CreateColorGradientStop();

    SetGradientAndStops();
    SetGradientStopColorsByTheme();
    m_rectangleGeometry->CornerRadius = safe_cast<float>(CornerRadius.TopLeft);
    auto spriteShape = m_compositor->CreateSpriteShape(m_rectangleGeometry);
    spriteShape->FillBrush = m_shimmerMaskGradient;
    m_shapeVisual->Shapes->Append(spriteShape);
    Windows::UI::Xaml::Hosting::ElementCompositionPreview::SetElementChildVisual(m_shape, m_shapeVisual);
    m_initialized = true;
    return true;
}

void ShimmerControl::SetGradientAndStops()
{
    m_shimmerMaskGradient->StartPoint = float2(InitialStartPointX, 0.0f);
    m_shimmerMaskGradient->EndPoint =   float2(0.0f, 1.0f);

    m_gradientStop1->Offset = 0.273f;
    m_gradientStop2->Offset = 0.436f;
    m_gradientStop3->Offset = 0.482f;
    m_gradientStop4->Offset = 0.643f;

    m_shimmerMaskGradient->ColorStops->Append(m_gradientStop1);
    m_shimmerMaskGradient->ColorStops->Append(m_gradientStop2);
    m_shimmerMaskGradient->ColorStops->Append(m_gradientStop3);
    m_shimmerMaskGradient->ColorStops->Append(m_gradientStop4);
}

void ShimmerControl::SetGradientStopColorsByTheme()
{
    switch (ActualTheme)
    {
    case ElementTheme::Default:
    case ElementTheme::Dark:
        m_gradientStop1->Color = Windows::UI::ColorHelper::FromArgb(static_cast<byte>(255 * 3.26 / 100), 255, 255, 255);
        m_gradientStop2->Color = Windows::UI::ColorHelper::FromArgb(static_cast<byte>(255 * 6.05 / 100), 255, 255, 255);
        m_gradientStop3->Color = Windows::UI::ColorHelper::FromArgb(static_cast<byte>(255 * 6.05 / 100), 255, 255, 255);
        m_gradientStop4->Color = Windows::UI::ColorHelper::FromArgb(static_cast<byte>(255 * 3.26 / 100), 255, 255, 255);
        break;
    case ElementTheme::Light:
        m_gradientStop1->Color = Windows::UI::ColorHelper::FromArgb(static_cast<byte>(255 * 5.37 / 100), 0, 0, 0);
        m_gradientStop2->Color = Windows::UI::ColorHelper::FromArgb(static_cast<byte>(255 * 2.89 / 100), 0, 0, 0);
        m_gradientStop3->Color = Windows::UI::ColorHelper::FromArgb(static_cast<byte>(255 * 2.89 / 100), 0, 0, 0);
        m_gradientStop4->Color = Windows::UI::ColorHelper::FromArgb(static_cast<byte>(255 * 5.37 / 100), 0, 0, 0);
        break;

    }
}

void ShimmerControl::UnRegisterDependencyProperties()
{
    s_animationDurationInMillisecondsProperty = nullptr;
    s_isAnimatingProperty = nullptr;

}

Windows::Foundation::TimeSpan ShimmerControl::TimeSpanFromMilliseconds(double const milliseconds)
{
    //NT_ASSERT(milliseconds >= 0);
    double const millisecondsToTimeSpan = 10000i64; // 100ns units
    return TimeSpan{ static_cast<int64>(milliseconds * millisecondsToTimeSpan) };
}

void ShimmerControl::OnApplyTemplate()
{
    Control::OnApplyTemplate();
    m_shape = static_cast<Border^>(GetTemplateChild("Shape"));
    if (m_shape == nullptr)
    {
        return;
    }
    if (m_initialized == false && TryInitializationResource() && IsAnimating)
    {
        TryStartAnimation();
    }
}
