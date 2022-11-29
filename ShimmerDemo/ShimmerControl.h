//
// ShimmerControl.h
// Declaration of the ShimmerControl class.
//

#pragma once

namespace ShimmerDemo
{
	public ref class ShimmerControl sealed : public Windows::UI::Xaml::Controls::Control
	{
	public:
		ShimmerControl();
		static property Windows::UI::Xaml::DependencyProperty^ AnimationDurationInMillisecondsProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get();
		}
		static property Windows::UI::Xaml::DependencyProperty^ IsAnimatingProperty
		{
			Windows::UI::Xaml::DependencyProperty^ get();
		}

		static void RegisterDependencyProperties();

		property bool IsAnimating
		{
			bool get();
			void set(bool value);
		}
		property double AnimationDurationInMilliseconds
		{
			double get();
			void set(double value);
		}

		static void UnRegisterDependencyProperties();
		

	protected:
		virtual void OnApplyTemplate() override;
	private:
		Windows::Foundation::TimeSpan TimeSpanFromMilliseconds(const double milliseconds);
		static Windows::UI::Xaml::DependencyProperty^ s_animationDurationInMillisecondsProperty;
		static Windows::UI::Xaml::DependencyProperty^ s_isAnimatingProperty;
		static void OnAnimationDurationInMillisecondsPropertyChanged(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
		static void OnIsAnimatingPropertyChanged(Windows::UI::Xaml::DependencyObject^ d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs^ e);
		void onLoaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnActualThemeChanged(FrameworkElement^, Object^);
		void TryStartAnimation();
		void StopAnimation();
		Windows::UI::Xaml::Controls::Border^ m_shape;
		bool m_initialized = false;
		bool m_isAnimationStarted = false;
		Windows::UI::Composition::CompositionRoundedRectangleGeometry^ m_rectangleGeometry;
		Windows::UI::Composition::Compositor^ m_compositor;
		Windows::UI::Composition::ShapeVisual^ m_shapeVisual;
		Windows::UI::Composition::CompositionLinearGradientBrush^ m_shimmerMaskGradient;
		Windows::UI::Composition::CompositionColorGradientStop^ m_gradientStop1;
		Windows::UI::Composition::CompositionColorGradientStop^ m_gradientStop2;
		Windows::UI::Composition::CompositionColorGradientStop^ m_gradientStop3;
		Windows::UI::Composition::CompositionColorGradientStop^ m_gradientStop4;
		bool TryInitializationResource();
		void SetGradientAndStops();
		void SetGradientStopColorsByTheme();
	};
}
